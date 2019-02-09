# RemoteRelay

Arduino code that controlls relays connected to the device. Relays can be opened and closed by HTTP calls. The device will update the corresponding openHAB switch item using the rest API whenever the relay is switched. The code currently supports up to 8 connected relays.

It allows to permanently switching the relay, or using a duration. If a duration is used, the end time is written to the flash memory and survives reboots.
In order for this to work, the code synchronizes the device time with an NTP server. It also opens a webserver that allows to view the current relay state and also see a debug log of things that have happened in the recent past.

## Installation

For installing the HW use any of the available tutorials (you will find instructions if you google for "ESP8266 relay")

Then compile and upload the fimware using your favorite Arduino IDE. You will need to following libraries:
    ArduinoJson
    Time
    Timezone
    WifiManager
    esp8266_mdns

## Setup

After installation, the device opens a WIFI Access Point. Connect to it to configure the WIFI name and password. Once the values are entered, the device will reboot and connect to the configured WIFI network.

Afterwards, go to the setup page to set up the device: http://\<device ip\>/setup

The setup page allows to configure the following things needed for proper operation of the remote relays:

* openHAB server: IP address of the openHAB server
* openHAB port: port of the openHAB server rest API
* NTP server: IP address of an NTP server
  
It additionally allows to configure the connected relays. You can set an item name of an openHAB switch item for every relay that is connected to one of the device's pins.

## Switching relays

An example call to ruen on relay connected to pin D1 for 20 seconds from the linux command line looks like this:
```
wget http://\<device ip\>/on?relay=1&duration=20
```
The duration parameter is optional and makes sure that the relay is turned off after the specfied duration in seconds. If you do not use the duration parameter, you can issue the following command to turn off the relay: 
```
wget http://\<device ip\>/off
```


If you want to use an openHAB switch item to control the relay, you have to use the http binding and define the switch items like this:

```
Switch IrrigationSwitchLawn "Bewässerung Rasen" { http=">[ON:GET:http://192.168.178.59/on?relay=1] >[OFF:GET:http://192.168.178.59/off?relay=1]",autoupdate="false"}
```

Make sure to adapt the IP address to the address of the device and use the correct relay number in the URLs.


## Checking device and relay state

Open the device in a web browser: http://\<device ip\>. This will show a page showing the status of time synchronization as well as the status of all connected relays.

If you have defined an item in openHAB like described above, the firmware will update the switch item whenever the relay is toggled.