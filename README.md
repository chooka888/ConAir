# ConAir
This repo contains the code for an airconditioner controller I made for Mitsubishi Heavy Industries airconditioners. 
I use ESP8266 microcontrollers to send commands to the airconditioners via IR. This repository contains: 
1. An andorid app that sends settings to the esp8266s via JSON over MQTT
2. The arduino code for the ESP8266s controllers. 

There is a bigger write up at Hackaday.io - https://hackaday.io/project/13279-esp8266-mqtt-infrared-aircon-control 

It is a work in progress, and at present the code is not cleaned up. It is basically proof of concept code, e.g. the android app runs the MQTT service in an activity so it is fragile, and doesn't survive an activity restart. There is some work to go, but better to get it under source control now. 
