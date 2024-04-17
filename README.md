# HuaweiSolarBridge
This is a small bridge arduino thing to enable a connection between Home Assistant and a Huwaei inverter. It is based on the information in https://github.com/wlcrs/huawei_solar/wiki/Connecting-to-the-inverter, section "Connect to the inverter AP (SUN2000-<serial_no> wifi)". I realized that you do not need a full wifi bridge, just a small forwarding gateway that will bridge one single TCP port from my home (IOT) wifi to the inverter AP. 

You need two ESP8266 wired together with RX and TX switched. And a power supply of course. I used two small ESP01 devices and 5V adapters.

The LAN server side contains a webserver that will show a simple log. Once you get the devices up and running, you can browse the ip and read this log. It is a good start if you can se the log.

An overview picture of the Idea
<img src="https://github.com/andcompe/HuaweiSolarBridge/blob/main/Overview.png" />


Here is a small picture of my circuit. Very simple.
<img src="https://github.com/andcompe/HuaweiSolarBridge/blob/main/Circuit.png" />


Note. This was not designed with IT Security in mind. It was created to see if it could be used to connect to my Inverter. It worked well, and has been working for several months without a single issue. But it is likely not hacker safe. Use at your own risk.
