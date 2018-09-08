#ifndef _RELAY_H
#define _RELAY_H 1

#include "Arduino.h"

#define OFF LOW
#define ON HIGH

class Relay {
    private:
    int state;
    long shutOffTime;

    public:
    int number;
    int pin;
    String item;

    private:
    void readShutOffTime();

    void writeShutOffTime();

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

void relay_setup();
void relay_loop();

#endif