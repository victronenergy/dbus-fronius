# This is a script for enumerating the SunSpec information models
# on a PV-inverter.
#
# Usage: python3 sunspec-enum.py IP_ADDRESS MODBUS_ADDRESS
#
# Eg: python3 sunspec-enum.py 192.168.0.1 126

import sys
from pymodbus.client.sync import ModbusTcpClient as ModbusClient

host = sys.argv[1]
unit = int(sys.argv[2]) if len(sys.argv) > 2 else 126

modbus = ModbusClient(host=host)
modbus.connect()

start = 40002 # Fronius starts at 40000, first two registers have 'Su' and 'nS'.
while True:
	r = modbus.read_holding_registers(start, 2, unit=unit)
	if r.registers[0] == 0xFFFF: break
	print ("model {} length {} at {}".format(r.registers[0], r.registers[1], start))
	start += r.registers[1] + 2
