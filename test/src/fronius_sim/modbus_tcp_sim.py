import bisect
import traceback
from pymodbus.server.asynchronous import ModbusSocketFramer, ModbusServerFactory
from pymodbus.datastore.context import ModbusServerContext, ModbusSlaveContext


class StringValue(object):
	def __init__(self, text, regcount):
		self.regcount = regcount
		self._text = text

	def get_value(self, offset):
		if offset >= self.regcount:
			return 0xFFFF
		chunk = self._text[2 * offset:2 * offset + 2]
		v0 = ord(chunk[0]) if len(chunk) > 0 else 0
		v1 = ord(chunk[1]) if len(chunk) > 1 else 0
		return v1 | (v0 << 8)


class IntValue(object):
	def __init__(self, value, regcount):
		self.regcount = regcount
		self._value = value

	def get_value(self, offset):
		if offset >= self.regcount:
			return 0xFFFF
		v = self._value
		for i in range(self.regcount - offset - 1):
			v = v >> 16
		return v & 0xFFFF


class DelegatedIntValue(object):
	def __init__(self, delegate, regcount, scale=1.0):
		self.regcount = regcount
		self._delegate = delegate
		self._scale = scale

	def get_value(self, offset):
		if offset >= self.regcount:
			return 0xFFFF
		v = int(self._delegate() * self._scale)
		for i in range(self.regcount - offset - 1):
			v = v >> 16
		return v & 0xFFFF


class CompositeValue(object):
	def __init__(self):
		self._offsets = []
		self._values = []

	def add_string(self, text, offset, regcount):
		self.add_value(StringValue(text, regcount), offset)

	def add_int(self, value, offset, regcount):
		self.add_value(IntValue(value, regcount), offset)

	def add_value(self, value, offset):
		i = bisect.bisect_left(self._offsets, offset)
		self._offsets.insert(i, offset)
		self._values.insert(i, value)

	def append_value(self, value):
		CompositeValue.add_value(self, value, self.regcount)

	@property
	def regcount(self):
		return self._offsets[-1] + self._values[-1].regcount

	def get_value(self, offset):
		i = bisect.bisect_left(self._offsets, offset)
		if i == len(self._offsets) or self._offsets[i] > offset:
			i -= 1
		if i < 0:
			return 0xFFFF
		return self._values[i].get_value(offset - self._offsets[i])


class SunspecModel(CompositeValue):
	def __init__(self, model_id):
		CompositeValue.__init__(self)
		self.model_id = model_id
		self.add_int(model_id, 0, 1)
		self.add_value(DelegatedIntValue(lambda: self.regcount - 2, 1), 1)


class FroniusDataBlock(CompositeValue):
	def __init__(self, inverter):
		CompositeValue.__init__(self)
		self._inverter = inverter

		self.add_string('SunS', 40000, 2)

		common_model = SunspecModel(1)
		common_model.add_string('Manufacturer', 2, 16)
		common_model.add_string('Model', 18, 16)
		common_model.add_string('Options', 34, 8)
		common_model.add_string('Version', 42, 8)
		common_model.add_string('Serial', 50, 16)
		common_model.add_int(1, 66, 1)  # modbus ID
		common_model.add_int(0xFFFF, 67, 1)  # Force even alignment
		self.append_value(common_model)

		inverter_model = SunspecModel(103)
		inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.main.current, 1, 10), 2)
		inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l1.current, 1, 10), 3)
		if self._inverter.has_3phases:
			inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l2.current, 1, 10), 4)
			inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l3.current, 1, 10), 5)
		inverter_model.add_int(-1, 6, 1)  # current scale factor
		inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l1.voltage, 1, 100), 10)
		if self._inverter.has_3phases:
			inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l2.voltage, 1, 100), 11)
			inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.l3.voltage, 1, 100), 12)
		inverter_model.add_int(-2, 13, 1)  # voltage scale factor
		inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.main.power, 1, 1), 14)
		inverter_model.add_int(0, 15, 1)  # power scale factor
		inverter_model.add_value(DelegatedIntValue(lambda: self._inverter.main.energy, 2, 10), 24)
		inverter_model.add_int(2, 26, 1)  # energy scale factor
		inverter_model.add_int(4, 38, 1)  # Operating state 4 ('MPPT')
		self.append_value(inverter_model)

		nameplate_model = SunspecModel(120)
		nameplate_model.add_value(DelegatedIntValue(lambda: self._inverter.max_power, 1, 0.1), 3)  # max power
		nameplate_model.add_int(1, 4, 1)  # max power scale
		self.append_value(nameplate_model)

		immediate_control_model = SunspecModel(123)
		if self._inverter.power_limit is not None:
			immediate_control_model.add_value(DelegatedIntValue(lambda: self._inverter.power_limit, 1, 100), 3)
			immediate_control_model.add_int(0, 7, 1)  # power limit timeout
			immediate_control_model.add_int(0, 9, 1)  # power limiter enabled
			immediate_control_model.add_int(-2, 23, 1)  # power limit scale
		self.append_value(immediate_control_model)

	def default(self, count, value=False):
		''' Used to initialize a store to one value

		:param count: The number of fields to set
		:param value: The default value to set to the fields
		'''
		print('default : {}, {}'.format(count, value))
		pass

	def reset(self):
		''' Resets the datastore to the initialized default value '''
		print('reset')
		pass

	def validate(self, address, count=1):
		''' Checks to see if the request is in range

		:param address: The starting address
		:param count: The number of values to test for
		:returns: True if the request in within range, False otherwise
		'''
		try:
			## print('validate', address, count)
			self.getValues(address, count)
			return True
		except IndexError:
			traceback.format_exc()
			return False

	def getValues(self, address, count=1):
		''' Returns the requested values from the datastore

		:param address: The starting address
		:param count: The number of values to retrieve
		:returns: The requested values from a:a+c
		'''
		try:
			v = [self.get_value(address + i - 1) for i in range(0, count)]
			## print(address, count, v)
			return v
		except:
			traceback.print_exc()

	def setValues(self, address, values):
		''' Returns the requested values from the datastore

		:param address: The starting address
		:param values: The values to store
		'''
		for v in values:
			self.set_value(address, v)

	def __str__(self):
		''' Build a representation of the datastore

		:returns: A string representation of the datastore
		'''
		return "DataStore(%d, %d)" % (len(self.values), self.default_value)

	def __iter__(self):
		''' Iterater over the data block data

		:returns: An iterator of the data block data
		'''
		# if isinstance(self.values, dict):
		# 	return self.values.iteritems()
		return enumerate(self._values)


def create_context(inverter):
	fdb = FroniusDataBlock(inverter)
	return ModbusSlaveContext(hr=fdb, it=fdb)


def start_server(inverters):
	from twisted.internet import reactor
	store = {int(i.id): create_context(i) for i in inverters if i.modbus_enabled}
	if len(store) == 0:
		return
	context = ModbusServerContext(slaves=store, single=False)
	factory = ModbusServerFactory(context, ModbusSocketFramer, None)
	reactor.listenTCP(5020, factory, interface='0.0.0.0')
