#ifndef _MAIN_H
#define _MAIN_H 1

#include <IPAddress.h>

const char *HOST = "RemoteRelay";
const char *OTA_PASS = "RemoteRelayPwd";
const char *AP_NAME = "RemoteRelayConfigAp";

// openHAB server
const char* OPENHAB2_SERVER = "192.168.178.35";
const int OPENHAB2_PORT = 8080;

// send updates to openHAB every xx seconds
const int UPDATE_INTERVAL = 60;

// NTP server
IPAddress timeServer(192, 168, 178, 1);

#define OFF LOW
#define ON HIGH

#endif