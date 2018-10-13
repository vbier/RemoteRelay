#include <FS.h> 
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "main.h"
#include "logging.h"
#include "time.h"
#include "webserver.h"

static char setupHtml[] PROGMEM = "<!DOCTYPE HTML>"
"<html lang='en'>"
"<head>"
"<title>RemoteRelay</title>"
"<meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>"
"<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>"
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.0/jquery.min.js'></script>"
"<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>"
"</head>"
"<body>"
"<div class='container'>"
"<h2>RemoteRelay Setup</h2>"
"<br />"
"<form action='/saveSetup' method='post'>"
"<h3>Connections</h3>"
"<div class='panel-body'>"
"<div class='form-group'>"
"<label for='openhab-server'>openHAB server</label>"
"<input type='text' required pattern='^([0-9]{1,3}\\.){3}[0-9]{1,3}$' class='form-control' id='openhab-server' name='openhab-server' placeholder='Enter IP address'>"
"<small id='openhab-server-help' class='form-text text-muted'>IP address of the openHAB server. Used to send relay state changes to openHAB.</small>"
"</div>"
"<div class='form-group'>"
"<label for='openhab-port'>openHAB port</label>"
"<input type='number' class='form-control' id='openhab-port' name='openhab-port' placeholder='Enter port number'>"
"<small id='openhab-port-help' class='form-text text-muted'>Port number under which the openHAB REST service can be reached, usually 8080.</small>"
"</div>"
"<div class='form-group'>"
"<label for='ntp-server'>NTP server</label>"
"<input type='text' required pattern='^([0-9]{1,3}\\.){3}[0-9]{1,3}$' class='form-control' id='ntp-server' name='ntp-server' placeholder='Enter IP address'>"
"<small id='ntp-server-help' class='form-text text-muted'>NTP server IP address. Used to synchronize the time to make sure relay switch durations work across reboots.</small>"
"</div>"
"</div>"
"<h3>Relays</h3>"
"<div class='panel-body'>"
"<div class='form-group'>"
"<label for='relayd0'>0: Relay Item connected to D0</label>"
"<input type='text' class='form-control' id='relayd0' name='relayd0' placeholder='Enter openHAB switch item name'>"
"<small id='relayd0-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D0.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd1'>1: Relay Item connected to D1</label>"
"<input type='text' class='form-control' id='relayd1' name='relayd1' placeholder='Enter openHAB switch item name'>"
"<small id='relayd1-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D1.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd2'>2: Relay Item connected to D2</label>"
"<input type='text' class='form-control' id='relayd2' name='relayd2' placeholder='Enter openHAB switch item name'>"
"<small id='relayd2-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D2.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd3'>3: Relay Item connected to D3</label>"
"<input type='text' class='form-control' id='relayd3' name='relayd3' placeholder='Enter openHAB switch item name'>"
"<small id='relayd3-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D3.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd4'>4: Relay Item connected to D4</label>"
"<input type='text' class='form-control' id='relayd4' name='relayd4' placeholder='Enter openHAB switch item name'>"
"<small id='relayd4-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D4.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd5'>5: Relay Item connected to D5</label>"
"<input type='text' class='form-control' id='relayd5' name='relayd5' placeholder='Enter openHAB switch item name'>"
"<small id='relayd5-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D5.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd6'>6: Relay Item connected to D6</label>"
"<input type='text' class='form-control' id='relayd6' name='relayd6' placeholder='Enter openHAB switch item name'>"
"<small id='relayd6-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D6.</small>"
"</div>"
"<div class='form-group'>"
"<label for='relayd7'>7: Relay Item connected to D7</label>"
"<input type='text' class='form-control' id='relayd7' name='relayd7' placeholder='Enter openHAB switch item name'>"
"<small id='relayd7-help' class='form-text text-muted'>The item name of the openHAB switch item that shall be controlled be the relay connected to pin D7.</small>"
"</div>"
"</div>"
"<button type='submit' class='btn btn-primary'>Submit</button>"
"</form>"
"<script>"
"(new URL(window.location.href)).searchParams.forEach((x, y) => document.getElementById(y).value = x)"
"</script>"
"</div>"
"</body>"
"</html>";

Config c = Config("", -1, IPAddress(0,0,0,0), {});

IPAddress toIPAddress(const char* text) {
    Serial.println("converting " + String(text) + " to IP address...");
    int ip[4];
    int count = sscanf(text, "%d.%d.%d.%d", ip, ip + 1, ip + 2, ip + 3);
    if (count == 4) {
        return IPAddress(ip[0], ip[1], ip[2], ip[3]);
    }

    return IPAddress(0,0,0,0);
}

Config* readConfig() {
    Serial.println("readConfig()");

    logMsg("reading configuration from SPIFFS...");
    if (SPIFFS.begin()) {
        if (SPIFFS.exists("/config.json")) {
            Serial.println("  config file exists");
            //file exists, reading and loading
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("  config file opened");
                size_t size = configFile.size();
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                configFile.close();

                DynamicJsonBuffer jsonBuffer;
                JsonObject& root = jsonBuffer.parseObject(buf.get());
                if (root.success()) {
                    root.printTo(Serial);

                    const char* server = root["server"];

                    c.openHABServer = String(server);
                    c.openHABPort = (int) root["port"];
                    c.timeServer = toIPAddress(root["ipaddress"]);

                    c.removeRelays();
                    int pins[8] = {D0, D1, D2, D3, D4, D5, D6, D7};
                    for (int i=0; i<8; i++) {
                        if (root.containsKey("relayd" + String(i))) {
                            const char* relayItem = root["relayd" + String(i)];
                            Relay r = Relay(i, pins[i], String(relayItem));
                            c.addRelay(r);
                        }
                    }

                    Serial.println("  configuration read from filesystem: " + c.toArgs());

                    return &c;
                }
            }
        }
    } 

    return nullptr;
}

bool saveConfig() {
    logMsg("writing configuration to SPIFFS...");
    if (SPIFFS.begin()) {
        File configFile = SPIFFS.open("/config.json", "w");
        if (configFile) {
            Serial.println("  config file opened");
            DynamicJsonBuffer jsonBuffer;

            JsonObject& root = jsonBuffer.createObject();
            root["server"] = c.openHABServer;
            root["port"] = c.openHABPort;
            root["ipaddress"] = c.timeServer.toString();

            for (Relay& r : *(c.getRelays())) {
                root["relayd" + String(r.number)] = r.item;
            }
            root.printTo(configFile);
            root.printTo(Serial);

            Serial.println("  config file written");
            return true;
        }
    }
    return false;
}

void setupHandler() {
    if (server.args() == 0 && config != nullptr) {
        // a call to setup without args, but we have a config, redirect and add args
        server.sendHeader("Location", "/setup?" + config->toArgs(),true); 
        server.send(302, "text/plain","");
        return;
    }
    server.send_P(200, "text/html", setupHtml);
}

void saveSetupHandler() {
    int pins[8] = {D0, D1, D2, D3, D4, D5, D6, D7};
    c.removeRelays();
    for (int i=0; i<8; i++) {
        if (server.hasArg("relayd" + String(i))) {
            String itemName = server.arg("relayd" + String(i));
            itemName.trim();

            if (itemName.length() > 0) {
                Relay r = Relay(i, pins[i], itemName);
                c.addRelay(r);
            }
        }
    }

    c.openHABServer = server.arg("openhab-server");
    c.openHABPort = server.arg("openhab-port").toInt();
    c.timeServer = toIPAddress(server.arg("ntp-server").c_str());

    if (saveConfig()) {
        time_setup();
        relay_setup();
        sendRedirect();
    } else {
        server.send_P(200, "text/html", "<htnl><body>Failed to save configuration.</body></html>");
    }
}

void config_setup() {
    server.on("/setup", setupHandler);
    server.on("/saveSetup", saveSetupHandler);

    config = readConfig();
}
