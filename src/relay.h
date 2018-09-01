#ifndef _RELAY_H
#define _RELAY_H 1

#include "Arduino.h"
#include "main.h"
#include <array>
#include "FS.h"
#include "time.h"
#include "logging.h"

class Relay {
    private:
    int state;
    long shutOffTime;

    public:
    int number;
    int pin;
    String item;

    private:
    void readShutOffTime() {
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

    void writeShutOffTime() {
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

    public:
    Relay(int n, int p, String i) {
        number = n;
        pin = p;
        item = i;
        state = OFF;

        readShutOffTime();
    }

    bool operator== ( Relay& other ) {
        return this == &other;
    }

    int getState() {
        return state;
    }

    void setState(int s) {
        state = s;
    }

    long getShutOffTime() {
        return shutOffTime;
    }

    void setShutOffTime(long time) {
        if (time != shutOffTime) {
            shutOffTime = time;
            writeShutOffTime();
        }
    }
};

std::vector<Relay> RELAYS = {Relay(1, D0, "IrrigationSwitchLawn"), Relay(2, D2, "IrrigationSwitchFlowers")};

Relay NO_RELAY(-1, -1, "");

std::vector<Relay> getRelays();

Relay* getRelayWithNumber(int num);

void initializePins();

#endif