#!/usr/bin/python3

import paho.mqtt.client as mqtt
import datetime
import logging
logging.basicConfig(level=logging.DEBUG)

def on_message(mosq, obj, msg):
    logging.info("received a message")
    ts = datetime.datetime.now(datetime.timezone.utc)
    ts_str = ts.astimezone().isoformat('T')
    print(msg)
    with open(ts_str+'.jpg', 'wb') as fd:
        fd.write(msg.payload)


client = mqtt.Client("itsme")
client.connect("localhost", 1883, 60)
client.on_message = on_message
client.subscribe("outTopic",0)

while True:
   client.loop(20)
