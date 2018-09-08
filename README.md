# RemoteRelay

Arduino code that controlls relay connected to the device. Relays can be opened and closed by HTTP calls. The device will update the corresponding openHAB switch item using the rest API whenever the relay is switched.

It allows to permanently switching the relay, or using a duration. If a duration is used, the end time is written to the flash memory and survives reboots.

The code synchronizes the device time with an NTP server and opens a webserver that allows to view the current relay state and also see a debug log of things that have happened in the recent past.

## Switching relays

An example call to open the second relay for 20 seconds from the linux command line looks like this:
```
wget http://\<device ip\>/on?relay=1&duration=20
```

The duration parameter is optional.

## Checking relay state

Open the device in a web browser.