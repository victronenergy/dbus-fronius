import random
import time


class PowerInfo:
	def __init__(self):
		self._lastEnergy = 0
		self._prevPower = 0
		# Use time.perf_counter() instead of time.clock() when using python 3
		self._lastTimeStamp = time.perf_counter()

	@property
	def current(self):
		return random.gauss(14, 0.5)

	@property
	def voltage(self):
		return random.gauss(230, 0.05)

	@property
	def power(self):
		p = random.gauss(3000, 100)
		t = time.perf_counter()
		self._lastEnergy += (self._prevPower + p) * (t - self._lastTimeStamp) / (2 * 3600)
		self._lastTimeStamp = t
		self._prevPower = p
		return p

	@property
	def nominal_power(self):
		return 2000

	@property
	def energy(self):
		p = self.power
		return self._lastEnergy


class FroniusSim:
	def __init__(self, id, unique_id, device_type, custom_name='', has_3phases=True, modbus_enabled=False,
			max_power=5000):
		self.main = PowerInfo()
		self.has_3phases = has_3phases
		self.modbus_enabled = modbus_enabled
		self.max_power = max_power
		self.power_limit = 100
		if has_3phases:
			self.l1 = PowerInfo()
			self.l2 = PowerInfo()
			self.l3 = PowerInfo()
		else:
			self.l1 = self.main
		self.id = id
		self.unique_id = unique_id
		self.custom_name = custom_name
		self.device_type = device_type
