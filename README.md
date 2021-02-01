# MeterCam

A WiFi "webcam" to monitor gas/water/electricity meters, or anything else that
needs regular (daily/weekly/monthly) inspection. When set to weekly intervals,
MeterCam will run on one set of AAA batteries for many years.


## Introduction

The MeterCam combines a standard ESP32-CAM module [1] (which works great, but
consumes a lot of power, even when sleeping) with custom hardware that only
powers up the camera module when it wants to take a picture, for example once
a day/week.

This custom hardware utilises a very energy-efficient "high-side power switch"
and a low-power microcontroller - keeping the power consumption during sleep
periods below 1 µW and resulting in battery lifetimes of many years.


## Features

- [ ] Configurable WiFi SSID/password
- [ ] Extended configuration options
- [ ] Configuration change via MQTT
- [ ] Image output via MQTT
- [ ] image acquisition on boot
- [ ] enable/disable "flash" LED
- [ ] Streaming video over HTTP (for setup)
- [ ] request shutdown


## Setup Instructions

The configuration web interface is based around XYZ's WifiManager[...].

A virgin MeterCam will enter configuration mode when first powered up.

Connect to the MeterCam WiFi Access Point (SSID "METERCAM", password:
"metercam") and configure all parameters as desired.

From now on, MeterCam will wake up at the configured intervals (see
[below](#operation)).

@todo you can force a configured MeterCam to go back into configuration mode
by setting jumper JPx. MeterCam will also fall back to configuration mode if
it hasn't been able to connect to the configured WiFi network five times in a
row (this is a feature of WifiManager).


## Operation

No maintenance is required during regular operation. MeterCam will

 1. wake up every interval
 2. connect to the configured WiFi
 3. take a picture
 4. transmit this picture via the configured means (MQTT, E-Mail, FTP, etc.)
 5. go back to sleep

@todo If MQTT is enabled, the MeterCam will subscribe to the configured "inbox" topic
and interpret incoming configuration requests.

## Detailed Description of MeterCam Components

### MeterCam Printed Circuit Board (PCB)

The MeterCam PCB contains a Microchip PIC16 microcontroller. It communicates
with the ESP32-CAM module via a bidirectional serial link. This link allows the
ESP32-CAM to request power-off and specific sleep durations.

The microcontroller also controls a "high-side power switch" used to cut power
to the ESP32-CAM module - thereby preventing the ESP32-CAM from consuming any
power during periods of inactivity.


### MeterCam µC Firmware

The software running on the MeterCam's microcontroller performs the following
tasks:

 - [ ] On initial bootup, switch *on* ESP32-CAM module, then wait for shutdown
   command .
 - [ ] Switch *off* ESP32-CAM module after 5 minutes, even if no
   (shutdown) command was received. Use a default sleep time of 1 day in this
   case.
 - [ ] Switch *off* ESP32-CAM when requested (via serial connection), and sleep
   for requested duration, consuming as little power as possible.
 - [ ] Switch *on* ESP32-CAM when requested sleep duration has passed.


### MeterCam ESP32-CAM Firmware

Compile using Arduino Studio with ESP extensions and the following libraries:
 - ESP32-CAM
 - [WifiManager](https://github.com/tzapu/WiFiManager)
 - -[DoubleResetDetect](https://github.com/jenscski/DoubleResetDetect)-
 - [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient), which requires...
   - [MQTT PubSubClient](https://github.com/knolleary/pubsubclient)



[1] ESP32-CAM module: ...
