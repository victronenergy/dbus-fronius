import random

class PowerInfo:
	def __init__(self):
		pass

	@property
	def current(self):
		return random.gauss(14, 0.5)

	@property
	def voltage(self):
		return random.gauss(230, 0.05)

	@property
	def power(self):
		return random.gauss(3373, 1000)


class FroniusSim:
	def __init__(self, id, unique_id, custom_name='', has_3phases=True):
		self.main = PowerInfo()
		self.has_3phases = has_3phases
		if has_3phases:
			self.l1 = PowerInfo()
			self.l2 = PowerInfo()
			self.l3 = PowerInfo()
		self.id = id
		self.unique_id = unique_id
		self.custom_name = custom_name
