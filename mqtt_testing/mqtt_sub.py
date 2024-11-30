import sys

import paho.mqtt.client as paho


def message_handling(client, userdata, msg):
    content = msg.payload.decode()
    print(f"{msg.topic}: {content}\ndatatype: {type(content)}")


client = paho.Client()
client.on_message = message_handling

if client.connect("localhost", 1883, 60) != 0:
    print("Couldn't connect to the mqtt broker")
    sys.exit(1)

client.subscribe("String")
client.subscribe("Int")

try:
    print("Press CTRL+C to exit...")
    client.loop_forever()
except Exception:
    print("Caught an Exception, something went wrong...")
finally:
    print("Disconnecting from the MQTT broker")
    client.disconnect()