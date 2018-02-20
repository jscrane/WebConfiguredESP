# WebConfiguredESP
Starter for sketches configurable by a JSON file in SPIFFS.

This starter takes care of the fiddly details of making your sketch
configurable via a web-browser by creating its own Access Point and
listening on [mDNS](https://en.wikipedia.org/wiki/Multicast_DNS).

It also supports OTA upgrade by web-browser.

## Software
- Arduino 1.8.4
- [ESP8266 for Arduino](https://github.com/esp8266/Arduino.git)
- [Arduino ESP8266 filesystem uploader](https://github.com/esp8266/arduino-esp8266fs-plugin)
- [ArduinoJson](http://arduinojson.org/)

## Installation
- Upload the filesystem (Tools > ESP8266 Sketch Data Upload)
- Upload the sketch

## Initialisation
Initially no WiFi is configured so it will go into AP mode:
- connect to SSID "MyESP"
- point a web-browser at http://192.168.4.1 or http://myesp.local
- configure your SSID and password
- update

Now it should connect to your network.

Debug over a serial port at 115200 baud.

## Credits
- Javascript [transparency](https://github.com/leonidas/transparency)
