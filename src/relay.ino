#include "relay.h"

std::vector<Relay> getRelays() {
    return RELAYS;
}

void initializePins() {
    for (auto r : RELAYS) {
        pinMode(r.pin, OUTPUT);
    }
}

Relay* getRelayWithNumber(int num) {
    for (std::vector<Relay>::iterator it = RELAYS.begin(); it != RELAYS.end(); ++it) {
        Relay & r = *it;

        if (r.number == num) {
            return &r;
        }
    }

    return &NO_RELAY;
}
