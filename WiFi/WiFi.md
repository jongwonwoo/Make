# WiFi

## ESP8266
* [ESP8266 Arduino Core](https://arduino-esp8266.readthedocs.io/en/2.4.0/index.html)
* [ESP8266WiFi library](https://arduino-esp8266.readthedocs.io/en/2.4.0/libraries.html#wifi-esp8266wifi-library)

### Wemos D1 mini
A mini wifi board with 4MB flash based on ESP-8266EX.
* [Specifications](https://wiki.wemos.cc/products:d1:d1_mini)
* Pins

| Pin | Function | ESP-8266 Pin |
| --- | --- | --- |
| TX | TXD | TXD |
| RX | RXD | RXD | 
| A0 | Analog input, max 3.3V input | A0 | 
| D0 | IO | GPIO16 |
| D1 | IO, SCL | GPIO5 |
| D2 | IO, SDA | GPIO4 |
| D3 | IO, 10k Pull-up | GPIO0 |
| D4 | IO, 10k Pull-up, BUILTIN_LED | GPIO2 |
| D5 | IO, SCK | GPIO14 |
| D6 | IO, MISO | GPIO12 |
| D7 | IO, MOSI | GPIO13 |
| D8 | IO, 10k Pull-down, SS | GPIO15 |
| G | Ground | GND |
| 5V | 5V | - |
| 3V3 | 3.3V | 3.3V |
| RST | Reset | RST |

### Using Arduino IDE
* Install Driver
	* [CH341 for macOS](http://www.wch.cn/download/CH341SER_MAC_ZIP.html)
	* [CH341 for Windows](http://www.wch.cn/download/CH341SER_ZIP.html)
* Install Hardware package
	* Open File → Preferences
	* Copy and paste **`http://arduino.esp8266.com/stable/packageesp8266comindex.json`** into Additional Boards Manager URLs field.
	* Open Tools → Board:xxx → Boards Manager and install **`esp8266 by ESP8266 Community`**
* Configure Board
	* Open Tools → Board:xxx and select **`WeMos D1 R2 & mini`**.
	* Open Tools → Flash Size and select **`4M (3M SPIFFS)`**.
	* Open Tools → CPU Frequency and select **`80 MHz`**.
	* Open Tools → Upload Speed and select **`115200`**.
* Plug board and select port
* Test your code
	* [Simple HTTP client](./wifitest)
