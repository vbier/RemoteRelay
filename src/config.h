#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <IPAddress.h>
#include "relay.h"

class Config {
    public:
        String openHABServer;
        int openHABPort;
        IPAddress timeServer;

        std::vector<Relay> relays;

        Config(String server, int port, IPAddress ntp, std::vector<Relay> rels) {
            openHABServer = server;
            openHABPort = port;
            timeServer = ntp;
        }

    bool operator== ( Config& other ) {
        return this == &other;
    }

    String toArgs() {
        String val = "openhab-server=" + openHABServer + "&openhab-port=" + String(openHABPort) + "&ntp-server=" + timeServer.toString();
        for(Relay& r : relays) {
            val += "&relayd" + String(r.number) + "=" + r.item;
        }

        return val;
    }

    std::vector<Relay>* getRelays() {
        return &relays;
    }

    void addRelay(Relay r) {
        relays.emplace_back(r);
    }
    
    void removeRelays() {
        relays.clear();
    }
};

Config* readConfig();

void config_setup();

#endif