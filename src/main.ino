/**
 * Remote control relays via embedded webserver.
 */
#include "Arduino.h"
#include <WiFiManager.h>

#include "main.h"
#include "logging.h"
#include "time.h"
#include "relay.h"
#include "ota.h"
#include "webserver.h"

const char *AP_NAME = "RemoteRelayConfigAp";

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, OFF);

    WiFiManager wifiManager;

    // Set configuration portal time out  - for 3 mins ( enough?)
    wifiManager.setConfigPortalTimeout(180);

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here "<HOST>ConfigAp"
    //and goes into a blocking loop awaiting configuration
    logMsg("trying to connect...");

    if (!wifiManager.autoConnect(AP_NAME)) {
        //if it is not able to connect - restart it and try again :)
        logMsg("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        logMsg("restarting ESP...");
        ESP.restart();
    }

    logMsg("checking wifi...");
    // if wlan fails we sit and wait the watchdog ;-)
    while (WiFi.status() != WL_CONNECTED) {
        delay(150);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
        digitalWrite(LED_BUILTIN, LOW);
    }
    digitalWrite(LED_BUILTIN, HIGH);
    logMsg("wifi connected.");

    // read config first
    config_setup();
    relay_setup();

    ota_setup();
    time_setup();    

    webserver_setup();
    logMsg("setup finished.");
}

void loop() {
    relay_loop();

    webserver_loop();
    time_loop();
    ota_loop();

    delay(100);
}