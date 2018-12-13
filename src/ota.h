#ifndef _OTA_H
#define _OTA_H 1

const char *OTA_PASS = "RemoteRelayPwd";

void ota_setup(const char* hostName, const char* passwd);
void ota_loop();

#endif