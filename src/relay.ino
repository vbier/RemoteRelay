#include "FS.h"
#include <unordered_set>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "main.h"
#include "time.h"
#include "logging.h"
#include "webserver.h"

long lastTime = -1;
Relay NO_RELAY(-1, -1, "");
std::unordered_set<int> pendingStateUpdates;

void Relay::readShutOffTime() {
    shutOffTime = -1;

    bool result = SPIFFS.begin();
    if (result) {
        File f = SPIFFS.open("/f" + String(pin) + ".txt", "r");
  
        if (f) {
            String line = f.readStringUntil('\n');
            f.close();   

            long storedTime = atol(line.c_str());
            logMsg("Read shutoff time for relay " + String(number) + " at pin " + String(pin) + ": " + line);

            if (storedTime > localTime() && isTimeSynchronized()) {
                shutOffTime = storedTime;
                state = ON;
            } 
        } 
    }
}

void Relay::writeShutOffTime() {
    bool result = SPIFFS.begin();
    if (result) {
        if (shutOffTime == -1 || shutOffTime < localTime()) {
            SPIFFS.remove("/f" + String(pin) + ".txt");
        } else {
            File f = SPIFFS.open("/f" + String(pin) + ".txt", "w");

            if (f) {
                f.println(String(shutOffTime));
                f.close();   
            } 
        }
    }
}


Relay* getRelayWithNumber(int num) {
    if (config != nullptr) {
        for (auto &r : *(config->getRelays())) {
            if (r.number == num) {
                return &r;
            }
        }
    }

    return &NO_RELAY;
}

int getRelayFromRequest() {
    if (server.hasArg("relay")) {
        String relayStr = server.arg("relay");
        return relayStr.toInt();
    }

    return -1;
}

int getDurationFromRequest() {
    if (server.hasArg("duration")) {
        String durationStr = server.arg("duration");
        int duration = durationStr.toInt();

        if (duration > 0) {
            return duration;
        }
    }

    return -1;
}

void offHandler() {
    int relay = getRelayFromRequest();
    Relay & r = *getRelayWithNumber(relay);

    if (r == NO_RELAY) {
        server.send(405, "text/plain", "No valid relay provided in the request");
    } else {
        logMsg("Received command OFF for relay " + String(r.number) + " at pin " + String(r.pin));
        setRelayState(&r, OFF);
        sendRedirect();
    }
}

void onHandler() {
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
        logMsg("relay state has been set, sending redirect");

        sendRedirect();
    }
}

void stateHandler() {
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
}

bool updateState(Relay r) {
    if (r.item.length() > 0 && config != nullptr) {
        Serial.println("updateState: relay " + String(r.number) + " at pin " + String(r.pin) + ", item=" + r.item);
       
        String value = "";

        switch (r.getState()) {
            case ON:
                value = "ON";
                break;
            case OFF:
                value = "OFF";
                break;
        }

        HTTPClient http;
        http.begin("http://" + config->openHABServer  + ":" + String(config->openHABPort) + "/rest/items/" + r.item + "/state");
        http.addHeader("Content-Type", "text/plain");
 
        int httpCode = http.PUT(value);
        Serial.println("Relay " + String(r.number) + " at pin " + String(r.pin) + ": httpCode= " + httpCode);

        if (httpCode < 200 || httpCode >= 300) {
            queueUpdate(r);
        } else {
            if (pendingStateUpdates.find(r.number) != pendingStateUpdates.end()) {
                pendingStateUpdates.erase(pendingStateUpdates.find(r.number));
            }
            logMsg("Sent state of relay " + String(r.number) + " at pin " + String(r.pin) + " (item " + r.item + ") to openHAB: " + value);
        }
        http.end();

        return httpCode >= 200 && httpCode < 300;
    }

    return false;
}

void queueUpdate(Relay r) {
    if(pendingStateUpdates.count(r.number) == 0) {
        logMsg("Queuing state update of relay " + String(r.number) + " at pin " + String(r.pin));
        pendingStateUpdates.emplace(r.number);
    } else {
        Serial.println("Relay " + String(r.number) + " at pin " + String(r.pin) + "already queued!");
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
    if (config == nullptr) {
        return;
    }

    for (auto &r : *(config->getRelays())) {
        boolean updateSent = false;
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

        if (!updateSent && (sendUpdates || (now % 10 == 0 && pendingStateUpdates.count(r.number)))) {
            Serial.println("trying to send buffered update for relay " + String(r.number));
            updateState(r);
        }
    }
}

void relay_setup() {
    if (config != nullptr) {
        logMsg("setting pin mode...");
        for (auto r : *(config->getRelays())) {
            pinMode(r.pin, OUTPUT);
        }
    }

    server.on("/on", onHandler);
    server.on("/off", offHandler);
    server.on("/state", stateHandler);
}

void relay_loop() {
    long now = localTime();

    // iterate relays only once per second
    if (now != lastTime) {
        // send state updates to openHAB on first iteration
        iterateRelays(now, lastTime == -1);
        lastTime = now;
    }
}