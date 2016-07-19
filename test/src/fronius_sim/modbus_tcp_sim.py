from pymodbus.server.async import StartTcpServer
from pymodbus.datastore.context import ModbusServerContext, ModbusSlaveContext


class FroniusDataBlock(object):
	def __init__(self, inverter):
		self._inverter = inverter

	def default(self, count, value=False):
		''' Used to initialize a store to one value

		:param count: The number of fields to set
		:param value: The default value to set to the fields
		'''
		print('default : {}, {}' % format(count, value))
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
			self.getValues(address, count)
			return True
		except IndexError:
			return False

	def getValues(self, address, count=1):
		''' Returns the requested values from the datastore

		:param address: The starting address
		:param count: The number of values to retrieve
		:returns: The requested values from a:a+c
		'''
		return [self._getValue(address + i) for i in range(0, count)]

	def setValues(self, address, values):
		''' Returns the requested values from the datastore

		:param address: The starting address
		:param values: The values to store
		'''
		for v in values:
			self._setValue(address, v)

	def _getValue(self, address):
		if address == 40084:
			return self._inverter.main.power
		if address == 40085:
			return 1
		if address == 40125:
			return self._inverter.main.nominal_power
		if address == 40126:
			return 1
		if address == 40233:
			return self._inverter.power_level
		if address == 40235:
			return 10
		if address == 40237:
			return self._inverter.power_throttle_enabled
		raise IndexError("address {} unknown".format(address))
	
	def _setValue(self, address, value):
		if address == 40233:
			self._inverter.power_level = value
# 		if address == 40235:
# 			return 10
		if address == 40237:
			self._inverter.power_throttle_enabled = value == 1
		raise IndexError("address {} unknown".format(address))

	def __str__(self):
		''' Build a representation of the datastore

		:returns: A string representation of the datastore
		'''
		return "DataStore(%d, %d)" % (len(self.values), self.default_value)

	def __iter__(self):
		''' Iterater over the data block data

		:returns: An iterator of the data block data
		'''
		if isinstance(self.values, dict):
			return self.values.iteritems()
		return enumerate(self.values, self.address)


def start_server(inverter):
	fdb = FroniusDataBlock(inverter)
	store = ModbusSlaveContext(hr=fdb, it=fdb)
	context = ModbusServerContext(slaves=store, single=True)
	StartTcpServer(context, address=('0.0.0.0', 5020))
