#include "FS.h"

#include "config.h"
#include "main.h"
#include "time.h"
#include "logging.h"
#include "webserver.h"

long lastTime = -1;
Relay NO_RELAY(-1, -1, "");

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

            if (storedTime > localTime()) {
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
        std::vector<Relay> relays = config->getRelays();
        for (std::vector<Relay>::iterator it = relays.begin(); it != relays.end(); ++it) {
            Relay & r = *it;

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

void updateState(Relay r) {
    if (r.item.length() > 0 && config != nullptr) {
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
        if (!client.connect(config->openHABServer, config->openHABPort)) { // connect with openhab 8080
            logMsg("Fehler: Verbindung zur OH konnte nicht aufgebaut werden");
            delay(100);
            return;
        }

        client.println("PUT /rest/items/" + r.item + "/state HTTP/1.1");
        client.print("Host: ");
        client.println(config->openHABServer);
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
    if (config == nullptr) {
        return;
    }

    std::vector<Relay> relays = config->getRelays();
    for (std::vector<Relay>::iterator it = relays.begin(); it != relays.end(); ++it) {
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

void relay_setup() {
    if (config != nullptr) {
        logMsg("setting pin mode...");
        for (auto r : config->getRelays()) {
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