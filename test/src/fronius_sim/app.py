import datetime
import os
import sys

app_dir = os.path.dirname(sys.argv[0])
bottle_dir = os.path.normpath(os.path.join(app_dir, '..', '..', '..', 'software', 'ext', 'bottle'))
print(bottle_dir)
sys.path.extend([bottle_dir, app_dir])

import bottle
from fronius_sim import FroniusSim

app_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
application = bottle.default_app()

inverters = [
    FroniusSim(id='1', unique_id='1234', has_3phases=True),
    FroniusSim(id='2', unique_id='4321', has_3phases=False)
]


@bottle.route('/solar_api/GetAPIVersion.cgi')
def get_api_version():
    return dict(APIVersion=1, BaseUrl='solar_api/v1/')


@bottle.route('/solar_api/v1/GetInverterInfo.cgi')
def get_inverter_info():
    return {
        'Head': create_head({}),
        'Body': {
            'Data': dict((x.id, {
                'DT': 192,
                'PVPower': 5000,
                'Show': 1,
                'UniqueID': x.unique_id,
                'ErrorCode': 0,
                'StatusCode': 7,
                'CustomName': 'SouthWest'})
                for x in inverters)}}


@bottle.route('/solar_api/v1/GetInverterRealtimeData.cgi')
def get_inverter_realtime_data():
    scope = bottle.request.query.Scope
    device_id = bottle.request.query.DeviceId
    data_collection = bottle.request.query.DataCollection
    if scope == 'Device':
        try:
            inverter = next((i for i in inverters if i.id == device_id))
        except StopIteration:
            return {
                'Head': create_head({
                    'Scope': scope,
                    'DeviceId': device_id,
                    'DataCollection': data_collection},
                    error_code=1,
                    error_message='device not found')}
        if data_collection == 'CumulationInverterData':
            return {
                'Head': create_head({
                    'Scope': scope,
                    'DeviceId': device_id,
                    'DataCollection': data_collection}),
                'Body': {
                    'Data': {
                        'PAC': {'Value': 3373, 'Unit': 'W'},
                        'DAY_ENERGY': {'Value': 8000, 'Unit': 'Wh'},
                        'YEAR_ENERGY': {'Value': 44000, 'Unit': 'Wh'},
                        'TOTAL_ENERGY': {'Value': 45000, 'Unit': 'Wh'},
                        'DeviceStatus': {
                            'StatusCode': 7,
                            'MgmtTimerRemainingTime': -1,
                            'ErrorCode': 0,
                            'LEDCode': 0,
                            'LEDColor': 2,
                            'LEDState': 0,
                            'StateToReset': False}}}}
        if data_collection == 'CommonInverterData':
            return {
                'Head': create_head({
                    'Scope': scope,
                    'DeviceId': device_id,
                    'DataCollection': data_collection}),
                'Body': {
                    'Data': {
                        'PAC': {'Value': inverter.main.power, 'Unit': 'W'},
                        'SAC': {'Value': 3413, 'Unit': 'VA'},
                        'IAC': {'Value': inverter.main.current, 'Unit': 'Hz'},
                        'UAC': {'Value': inverter.main.power, 'Unit': 'V'},
                        'FAC': {'Value': 50, 'Unit': 'Hz'},
                        'IDC': {'Value': 8.2, 'Unit': 'A'},
                        'UDC': {'Value': 426, 'Unit': 'V'},
                        'DAY_ENERGY': {'Value': 8000, 'Unit': 'Wh'},
                        'YEAR_ENERGY': {'Value': 44000, 'Unit': 'Wh'},
                        'TOTAL_ENERGY': {'Value': 45000, 'Unit': 'Wh'},
                        'DeviceStatus': {
                            'StatusCode': 7,
                            'MgmtTimerRemainingTime': -1,
                            'ErrorCode': 0,
                            'LEDCode': 0,
                            'LEDColor': 2,
                            'LEDState': 0,
                            'StateToReset': False}}}}
        if data_collection == '3PInverterData':
            if not inverter.has_3phases:
                return {
                    'Head': create_head({
                        'Scope': scope,
                        'DeviceId': device_id,
                        'DataCollection': data_collection},
                        error_code=2,
                        error_message='not supported')}
            return {
                'Head': create_head({
                    'Scope': scope,
                    'DeviceId': device_id,
                    'DataCollection': data_collection}),
                'Body': {
                    'Data': {
                        'IAC_L1': {'Value': inverter.l1.current, 'Unit': 'A'},
                        'IAC_L2': {'Value': inverter.l2.current, 'Unit': 'A'},
                        'IAC_L3': {'Value': inverter.l3.current, 'Unit': 'A'},
                        'UAC_L1': {'Value': inverter.l1.voltage, 'Unit': 'V'},
                        'UAC_L2': {'Value': inverter.l2.voltage, 'Unit': 'V'},
                        'UAC_L3': {'Value': inverter.l3.voltage, 'Unit': 'V'},
                        'T_AMBIENT': {'Value': 27, 'Unit': 'V'},
                        'ROTATION_SPEED_FAN_FL': {'Value': 83, 'Unit': 'RPM'},
                        'ROTATION_SPEED_FAN_FR': {'Value': 83, 'Unit': 'RPM'},
                        'ROTATION_SPEED_FAN_BL': {'Value': 83, 'Unit': 'RPM'},
                        'ROTATION_SPEED_FAN_BR': {'Value': 83, 'Unit': 'RPM'}}}}
    elif scope == 'System':
        return {
            'Head': create_head({'Scope': scope}),
            'Body': {
                'Data': {
                    'PAC': {'Value': 3373, 'Unit': 'W'},
                    'DAY_ENERGY': {'Value': 8000, 'Unit': 'Wh'},
                    'YEAR_ENERGY': {'Value': 44000, 'Unit': 'Wh'},
                    'TOTAL_ENERGY': {'Value': 45000, 'Unit': 'Wh'}}}}
    else:
        raise Exception('Unknown scope')


def create_head(args, error_code=0, error_message=''):
    return {
        'RequestArguments': args,
        'Status': {
            "Code": error_code,
            "Reason": error_message,
            "UserMessage": ""},
        'Timestamp': datetime.datetime.now().isoformat()}


if __name__ == '__main__':
    # host='0.0.0.0': accept connections from all sources
    bottle.run(host='0.0.0.0', port=8080, debug=True)
