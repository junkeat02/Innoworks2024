import datetime
import time
import sys
# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.append('C:/Users/USER/Documents/Innoworks/')
from serial_com.serial_connection import SerialConnection
from weather_api.request_api import WeatherAPI

from wisepaasdatahubedgesdk.EdgeAgent import EdgeAgent
import wisepaasdatahubedgesdk.Common.Constants as constant
from wisepaasdatahubedgesdk.Model.Edge import EdgeAgentOptions, MQTTOptions, DCCSOptions, EdgeData, EdgeTag, EdgeStatus, EdgeDeviceStatus, EdgeConfig, NodeConfig, DeviceConfig, AnalogTagConfig, DiscreteTagConfig, TextTagConfig
from wisepaasdatahubedgesdk.Common.Utils import RepeatedTimer


serial_port_num = 8
class DatahubConnection:
    def __init__(self):
        self.connection_status = False
        DCCS_API_URL = "https://api-dccs-ensaas.education.wise-paas.com/"
        CREDENTIAL_KEY = "bddd3bf72a67d2aada0132cd40d40b2j"
        EDGE_TYPE = "Gateway"
        NODE_ID = "32d1eac8-20fe-4852-b328-07e4f0fb9c0c"
        self.DEVICE_ID = "2xxlTRJauVAO"
        CONNECT_TYPE = ["MQTT", "DCCS"]
        self.options = EdgeAgentOptions(
            reconnectInterval = 1,                                # MQTT reconnect interval in seconds.
            nodeId = NODE_ID,      # Get from portal
            deviceId = self.DEVICE_ID,                                 # If the type is Device, deviceId must be input. 
            type = constant.EdgeType[EDGE_TYPE],                  # Configure the edge as a Gateway or Device. The default setting is Gateway.
            heartbeat = 60,                                       # The default is 60 seconds.
            dataRecover = True,                                   # Whether to recover data when disconnected
            connectType = constant.ConnectType[CONNECT_TYPE[0]],           # Connection type (DCCS, MQTT). The default setting is DCCS.
            MQTT = MQTTOptions(                                   # If the connectType is MQTT, the following options must be input:
                hostName = 'rabbitmq-pub.education.wise-paas.com',
                port = 1883,
                userName = 'DXQBg2rk7keK:Ojy1L9OK23qk',
                password = 'BBOHhgc2GrYZitqjcHWa',
                # protocolType = constant.Protocol['TCP']             # MQTT protocol (TCP, WebSocket). The default is TCP.
            ),
            DCCS = DCCSOptions(
                apiUrl = DCCS_API_URL,         # DCCS API URL
                credentialKey = CREDENTIAL_KEY  # Credential key
            )
        )
        self.edgeAgent = EdgeAgent(options = self.options)
        
        self.edgeAgent.on_connected = self.is_connected
        self.edgeAgent.on_disconnected = self.disconnected
        self.edgeAgent.on_message = self.on_message
        
        self.edgeData = EdgeData()
        self.config = EdgeConfig()
        # set node config
        # set device config
        # set tag config

        
    def is_connected(self, agent, isConnected):
        if isConnected:
            self.connection_status = True
            print("Connection success")
            
    def disconnected(self, agent, isDisconnected):
        if isDisconnected:
            self.connection_status = False
            print("Disconnected")
            
    def on_message(self, agent, messageReceivedEventArgs):
        # messageReceivedEventArgs format: Model.Event.MessageReceivedEventArgs
        type = messageReceivedEventArgs.type
        message = messageReceivedEventArgs.message
        if type == constant.MessageType['WriteValue']:
            # message format: Model.Edge.WriteValueCommand
            for device in message.deviceList:
                print('deviceId: {0}'.format(device.Id))
                for tag in device.tagList:
                    print('tagName: {0}, Value: {1}'.format(tag.name, str(tag.value)))
        elif type == constant.MessageType['WriteConfig']:
            print('WriteConfig')
        elif type == constant.MessageType['TimeSync']:
            # message format: Model.Edge.TimeSyncCommand
            print(str(message.UTCTime))
        elif type == constant.MessageType['ConfigAck']:
            # message format: Model.Edge.ConfigAck
            print('Upload Config Result: {0}'.format(str(message.result)))
            
    def connect_to_datahub(self):
        try:
            self.edgeAgent.connect()  # Ensure you're calling connect()
            print("Attempting connection...")
        except Exception as e:
            print(f"Error during connection setup: {e}")
    
    def textConfiguration(self, config_index, tag_name):
        action_type = ["Create", "Update", "Delete"]
        nodeConfig = NodeConfig(nodeType = constant.EdgeType['Gateway'])
        self.config.node = nodeConfig
        
        deviceConfig = DeviceConfig(
            id = self.DEVICE_ID,
            name = 'ESP32',
            deviceType = 'Microcontroller',
            description = ''
        )
        
        textTag = TextTagConfig(
            name = tag_name,
            description = '',
            readOnly = False,
            arraySize = 0
        )
        
        deviceConfig.textTagList.append(textTag)
        self.config.node.deviceList.append(deviceConfig)
        self.config.node.deviceList[0].textTagList.append(textTag)
        result = self.edgeAgent.uploadConfig(constant.ActionType[action_type[config_index]], edgeConfig = self.config)
        print(f"Analog configuration: {result}")
        return result
    
    def analogConfiguration(self, config_index, tag_name, array_size):
        action_type = ["Create", "Update", "Delete"]
        nodeConfig = NodeConfig(nodeType = constant.EdgeType['Gateway'])
        self.config.node = nodeConfig
        
        deviceConfig = DeviceConfig(
            id = self.DEVICE_ID,
            name = 'ESP32',
            deviceType = 'Microcontroller',
            description = ''
        )
        
        # textTag = TextTagConfig(
        #     name = tag_name,
        #     description = '',
        #     readOnly = False,
        #     arraySize = 0
        # )
        unit = ""
        if "current" in tag_name:
            unit = "mA"
        elif "power" in tag_name:
            unit = "W"
        analogTag = AnalogTagConfig(
            name = tag_name,
            description = '',
            readOnly = False,
            arraySize = array_size,
            spanHigh = 1000,
            spanLow = 0,
            engineerUnit = unit,
            integerDisplayFormat = 2,
            fractionDisplayFormat = 4
        )
        
        # deviceConfig.textTagList.append(textTag)
        deviceConfig.analogTagList.append(analogTag)
        self.config.node.deviceList.append(deviceConfig)
        # self.config.node.deviceList[0].textTagList.append(textTag)
        self.config.node.deviceList[0].analogTagList.append(analogTag)
        result = self.edgeAgent.uploadConfig(constant.ActionType[action_type[config_index]], edgeConfig = self.config)
        print(f"Analog configuration: {result}")
        return result
    
    def sendSingleData(self, tagName, value):
        tag = EdgeTag(self.DEVICE_ID, tagName, value)
        self.edgeData.tagList.append(tag)
        self.edgeData.timestamp = datetime.datetime.now()  #edgeData.timestamp = datetime.datetime(2020,8,24,6,10,8)  	# You can specify the time(local time) of data

        result = self.edgeAgent.sendData(data = self.edgeData)
        self.edgeData.tagList.clear()
        # self.configuration(0, tag_name)
        return result
    
    def sendAnalogData(self, tagName, dicVal):
        array = []
        tag = EdgeTag(deviceId = self.DEVICE_ID, tagName = tagName, value = dicVal)
        self.edgeData.tagList.append(tag)
        self.edgeData.timestamp = datetime.datetime.now()  #edgeData.timestamp = datetime.datetime(2020,8,24,6,10,8)  	# You can specify the time(local time) of data
        array.append(self.edgeData)
        result = self.edgeAgent.sendData(array)
        self.edgeData.tagList.clear()
        return result
    
    def sendAnalogDataArray(self, tagName, dicVal):
        array = []
        tag = EdgeTag(deviceId = self.DEVICE_ID, tagName = tagName, value = dicVal)
        self.edgeData.tagList.append(tag)
        self.edgeData.timestamp = datetime.datetime.now()  #edgeData.timestamp = datetime.datetime(2020,8,24,6,10,8)  	# You can specify the time(local time) of data
        array.append(self.edgeData)
        result = self.edgeAgent.sendData(array)
        self.edgeData.tagList.clear()
        return result
    
# external variables
waiting_connection_period = 1  # in second
data_sending_period = 0.2  # in second
last_time = time.time()
n = 0
connected = False
no_of_poles = 2
tag_name_list = []
coor = (5.9, 100.29)
api = WeatherAPI(coor[0], coor[1])
api.initialize()
weather_tag_name = ["temp", "hum", "weather"]
weather_methods = [api.get_temp(), api.get_humidity(), api.get_weather()['description']]

if __name__ == "__main__":
    while(1):
        try:
            sr = SerialConnection(serial_port_num)
            sr.start_connection()
            
        except Exception as e:
            print(f"Please connect your esp32 and check the port connected.")
            time.sleep(1)
            continue
        else:
            app = DatahubConnection()
            keys = sr.keys[1:]
            print(keys)
            break
    configured = False
            
                
    while True:
        if not app.connection_status:
            app.connect_to_datahub()
            time.sleep(waiting_connection_period)  # Waiting period for the connection to be established
        if not configured:
            # Lamp conditions
            for y in range(no_of_poles):
                print(f"keys: {keys}")
                for x in keys:
                    tag_name = f"{x}{y}"
                    tag_name_list.append(tag_name)
                    app.analogConfiguration(0, tag_name, 0)
            print(tag_name_list)
            for x in range(len(weather_tag_name)-1):
                app.analogConfiguration(0, weather_tag_name[x], 0)
            # print(f"Weather tag name: {weather_tag_name}")
            app.textConfiguration(0, weather_tag_name[-1])
                
            
            configured = True
        # print(f"tag name list: {tag_name_list}")
        if time.time() - last_time > data_sending_period and app.connection_status:
            try:
                recv_data = sr.read_data()
                print(recv_data)
            except Exception:
                continue
            
            # value = {"0": recv_data["current"], "1": recv_data["solar_voltage"], "2": recv_data["day_time"], "3": recv_data["rain_detection"]}
            if isinstance(recv_data['position'], list):
                print(len(recv_data["position"]))
                n = 0
                for y in range(len(recv_data["position"])):
                    n += y * len(keys)
                    print(n)
                    for x in range(int(len(tag_name_list)/no_of_poles)):
                        send_status = app.sendAnalogDataArray(tagName=tag_name_list[x + n], dicVal=recv_data[keys[x]][y])
                        if not send_status:
                            print(f"Lamp status delivery: {send_status}")
                for x in range(len(weather_tag_name)):
                    # print(len(weather_tag_name))
                    # print(f"weather methods: {weather_methods}")
                    send_status = app.sendAnalogDataArray(tagName=weather_tag_name[x], dicVal=weather_methods[x])
                    if not send_status:
                        print(f"Weather status delivery: {send_status}")
                # send_status = app.sendAnalogDataArray(tagName=tag_name_list[0], dicVal=recv_data[keys[0]])
                # print(f"Data delivery status: {send_status}")
            last_time = time.time()
                # print(recv_data)

    