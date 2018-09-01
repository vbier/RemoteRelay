/**
 * Remote control relays via embedded webserver.
 */
#include "Arduino.h"

#include "main.h"
#include "logging.h"
#include "time.h"
#include "relay.h"
#include "ota.h"
#include "webserver.h"
#include <WiFiManager.h>

long lastTime = -1;

void updateState(Relay r) {
    if (r.item.length() > 0) {
        logMsg("Sending state of relay " + String(r.number) + " at pin " + String(r.pin) + " (item " + r.item + ") to openHAB: " + String(r.getState()));

        String value = "";
        String vlength = "";

        switch (r.getState()) {
            case ON:
                value = "ON";
                vlength = "2";
                break;
            case OFF:
                value = "OFF";
                vlength = "3";
                break;
        }

        WiFiClient client; // Webclient initialisieren
        if (!client.connect(OPENHAB2_SERVER, OPENHAB2_PORT)) { // connect with openhab 8080
            logMsg("Fehler: Verbindung zur OH konnte nicht aufgebaut werden");
            delay(100);
            return;
        }

        client.println("PUT /rest/items/" + r.item + "/state HTTP/1.1");
        client.print("Host: ");
        client.println(OPENHAB2_SERVER);
        client.println("Connection: close");
        client.println("Content-Type: text/plain");
        client.println("Content-Length: " + vlength + "\r\n");
        client.print(value);
    }
}

void setRelayState(Relay *relay, int state) {
    Relay & r = *relay;

    if (state == OFF) {
        r.setShutOffTime(-1);
    }

    if (r.getState() != state) {
        r.setState(state);
        updateState(r);
    }
}

void iterateRelays(long now, boolean sendUpdates) {
    for (std::vector<Relay>::iterator it = RELAYS.begin(); it != RELAYS.end(); ++it) {
        boolean updateSent = false;

        Relay & r = *it;
        //logMsg("relay " + String(r.number) + " at pin " + String(r.pin) + ": state=" + String(r.getState()) + ", time=" + String(r.getShutOffTime()));

        if (r.getState() == ON && r.getShutOffTime() > 0 && r.getShutOffTime() < now && digitalRead(r.pin) == ON) {
            logMsg("end time reached. setting relay state to OFF");
            setRelayState(&r, OFF);
            updateSent = true;
        }

        if (r.getState() == OFF && digitalRead(r.pin) == ON) {
            logMsg("switching off relay " + String(r.number) + " at pin " + String(r.pin));
            digitalWrite(r.pin, OFF);
        }

        if (r.getState() == ON && digitalRead(r.pin) == OFF) {
            logMsg("switching on relay " + String(r.number) + " at pin " + String(r.pin));
            digitalWrite(r.pin, ON);
        }

        if (!updateSent && sendUpdates) {
            updateState(r);
        }
    }
}

void loop() {
    long now = localTime();

    // iterate relays only once per second
    if (now != lastTime) {
        // send state updates to openHAB on first iteration
        iterateRelays(now, lastTime == -1);
        lastTime = now;
    }

    webserver_loop();
    time_loop();
    ota_loop();

    delay(100);
}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, OFF);

    logMsg("setting pin mode...");
    initializePins();

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

    ota_setup(HOST, OTA_PASS);
    webserver_setup([]() {
        int relay = getRelayFromRequest();
        Relay & r = *getRelayWithNumber(relay);

        if (r == NO_RELAY) {
            server.send(405, "text/plain", "No valid relay provided in the request");
        } else {
            int duration = getDurationFromRequest();
            if (duration > 0) {
                logMsg("Received command ON with duration " + String(duration) + " for relay " + String(r.number) + " at pin " + String(r.pin));

                long now = localTime();
                r.setShutOffTime(now + duration);
            } else {
                logMsg("Received command ON for relay " + String(r.number) + " at pin " + String(r.pin));
                r.setShutOffTime(-1);
            }

            setRelayState(&r, ON);
            sendRedirect();
        }
    },[]() {
        int relay = getRelayFromRequest();
        Relay & r = *getRelayWithNumber(relay);

        if (r == NO_RELAY) {
            server.send(405, "text/plain", "No valid relay provided in the request");
        } else {
            logMsg("Received command OFF for relay " + String(r.number) + " at pin " + String(r.pin));
            setRelayState(&r, OFF);
            sendRedirect();
        }
    },[]() {
        int relay = getRelayFromRequest();
        Relay & r = *getRelayWithNumber(relay);

        if (r == NO_RELAY) {
            server.send(405, "text/plain", "No valid relay provided in the request");
        } else {
            String s;
            if (r.getState() == ON) {
                s = "{\n   \"state\":ON\n}";
            } else {
                s = "{\n   \"state\":OFF\n}";
            }

            server.send(200, "text/plain", s);
        }
    });
    time_setup();
    
    logMsg("setup finished.");
}