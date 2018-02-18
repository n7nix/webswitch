# Internet based AC relays/switches

## Switches from Digital Loggers, Inc (dli) IP power control
* [Digtal Loggers home web page](https://dlidirect.com/)

#### IOT Power Relay
* Not really an Internet relay but easily controlled by a Raspberry Pi
* [IoT Power Relay](https://dlidirect.com/products/iot-power-relay) has:
  * 2 normally off switchable outlets
  * 1 normally on switchable outlet
  * 1 always on outlet
* Three switchable outlets controlled by single 3.3v RPi GPIO
  * Control voltage is 3 - 60VDC

* [bash script](https://github.com/n7nix/webswitch/blob/master/dli/iot_power_relay/gpiodli_relay.sh) runs from a Raspberry Pi with a NW Digital Radio UDRC HAT
  * Identifies available GPIOs and toggles them off/on to control relay


#### Web Power Switch

* [C code](https://github.com/n7nix/webswitch/tree/master/dli/web_power_switch) for older DLI Web Power Switch that uses [cURL](https://en.wikipedia.org/wiki/CURL) to control one of eight outlets
* Web Switch has:
  * 8 independent switchable outlets & 2 always-on outlets
* Controlled by web interface ([cURL](https://en.wikipedia.org/wiki/CURL))

#### DLI products in Web Power Switch product line
* The code here is used with an older Power Switch which is no longer sold.
* The [Web Power Switch 7](https://dlidirect.com/products/web-power-switch-7), is EOL or just sold out? (Feb 2018)
* Replacement to Web Power Switch is [Pro Switch](https://dlidirect.com/products/new-pro-switch)?

## Switches from Sainsmart
* Not really an Internet relay but easily controlled by a Raspberry Pi

#### USB Relay modules
* [8-channel 12V USB Relay Module](https://www.sainsmart.com/products/8-channel-12v-usb-relay-module)
* See [CRELAY](http://ondrej1024.github.io/crelay/), Controlling different relay cards for home automation with a Linux software on Github
  * [Code here](https://github.com/n7nix/webswitch/tree/master/sainsmart) is based on CRELAY:
    * simplified cli C program that uses Sainsmart relay driver
    * script to setup udev to allow running usb ftdi device as some user rather than root
