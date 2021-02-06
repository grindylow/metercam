#!/usr/bin/python3

import paho.mqtt.client as mqtt
import datetime
import logging
logging.basicConfig(level=logging.DEBUG)

def on_jpg_message(mosq, obj, msg):
    logging.info("received a jpg message")
    ts = datetime.datetime.now(datetime.timezone.utc)
    ts_str = ts.astimezone().isoformat('T').replace(':','')
    s = msg.topic.replace('/','_')
    with open(s+'_'+ts_str+'.jpg', 'wb') as fd:
        fd.write(msg.payload)

def on_info_message(mosq, obj, msg):
    logging.info("received an info message")
    logging.info("publishing sleep_intervals request")
    mosq.publish("metercam/DEADBEEF/inbox", '{ "sleep_intervals": 0 }')

client = mqtt.Client("itsme")
client.connect("localhost", 1883, 60)
#client.on_message = on_jpg_message
client.subscribe("metercam/+/jpg",0)
client.subscribe("metercam/+/info",0)
client.message_callback_add("metercam/+/info", on_info_message)
client.message_callback_add("metercam/+/jpg", on_jpg_message)

while True:
   client.loop(20)
