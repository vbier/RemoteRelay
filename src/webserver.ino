#include "webserver.h"
#include "main.h"
#include "time.h"
#include "config.h"
#include "logging.h"
#include "Arduino.h"

static char refreshHtml[] PROGMEM =
    "<!DOCTYPE HTML>\n"
    "<html lang='en'>\n"
    "<head>\n"
    "<meta http-equiv='refresh' content=\"0; url='/'\"/>\n"
    "</head>\n"
    "</html>";

static char webSiteHtml[] PROGMEM =
    "<!DOCTYPE HTML>\n"
    "<html lang='en'>\n"
    "<head>\n"
    "<title>RemoteRelays</title>\n"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>\n"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>\n"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>\n"
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.0/jquery.min.js'></script>\n"
    "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>\n"
    "<SCRIPT>\n"
    "var xmlHttp=createXmlHttpObject();\n"
    "var titleSet=false;\n"
    "function createXmlHttpObject(){\n"
    " if(window.XMLHttpRequest){\n"
    "    xmlHttp=new XMLHttpRequest();\n"
    " }else{\n"
    "    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n"
    " }\n"
    " return xmlHttp;\n"
    "}\n"
    "function process(){\n"
    " if(xmlHttp.readyState==0 || xmlHttp.readyState==4){\n"
    "   xmlHttp.open('PUT','xml',true);\n"
    "   xmlHttp.onreadystatechange=handleServerResponse;\n"
    "   xmlHttp.send(null);\n"
    " }\n"
    " setTimeout('process()',1000);\n"
    "}\n"
    "function handleServerResponse(){\n"
    " if(xmlHttp.readyState==4 && xmlHttp.status==200){\n"
    "   xmlResponse=xmlHttp.responseXML;\n"
    "   if (!titleSet) {\n"
    "     xmldoc = xmlResponse.getElementsByTagName('id');\n"
    "     message = xmldoc[0].firstChild.nodeValue;\n"
    "     document.title = message;\n"
    "     document.getElementById('pageTitle').innerHTML=message;\n"
    "     titleSet=true;\n"
    "   }\n"
    "   xmldoc = xmlResponse.getElementsByTagName('status');\n"
    "   message = xmldoc[0].firstChild.nodeValue;\n"
    "   document.getElementById('status').innerHTML=message;\n"
    "   xmldoc = xmlResponse.getElementsByTagName('debug');\n"
    "   message = xmldoc[0].firstChild.nodeValue;\n"
    "   document.getElementById('debug').innerHTML=message.replace(/\\n/g, \"<br/>\");\n"
    " }\n"
    "}\n"
    "</SCRIPT>\n"
    "</head>\n"
    "<BODY onload='process()'>\n"
    " <div class='container'>\n"
    "  <h2 id='pageTitle'>RemoteRelay</h2>\n"
    "  <br />"
    "  <div class='panel panel-default'>\n"
    "   <div class='panel-heading'>Status</div>\n"
    "   <div class='panel-body' id='status'><br/><br/><br/></div>\n"
    "  </div>\n"
    "  <br />"
    "  <div class='panel-group'>\n"
    "   <div class='panel panel-default'>\n"
    "    <div class='panel-heading'>\n"
    "      <a data-toggle='collapse' href='#collapse1'>Debug Log</a>\n"
    "    </div>\n"
    "    <div id=\"collapse1\" class=\"panel-collapse collapse\">"
    "     <div class='panel-body' id='debug'><br/><br/><br/><br/><br/></div>\n"
    "    </div>\n"
    "   </div>\n"
    " </div>\n"
    "</BODY>\n"
    "</HTML>\n";

String generateXML() {
    String XML = "<?xml version='1.0'?>\n";
    XML += "<response>";
    XML += "<id>";
    XML += HOST;
    XML += "</id>";
    XML += "<status>Time synchronized: ";
    XML += (isTimeSynchronized() ? "true" : "false");
    XML += "&lt;br/&gt;";

    if (config != nullptr) {
        for(Relay& r : config->getRelays()) {
            XML += "Status Relay " + String(r.number) + ": ";
            if (r.getState() == ON) {
                XML += "ON";
            } else {
                XML += "OFF";
            }
            XML += "&lt;br/&gt;";
        }
    }

    XML += "Aktualisierung am: ";
    XML += getFormattedTime();
    XML += "</status>";

    XML += "<debug><![CDATA[";
    XML += getLogText();
    XML += "]]></debug>";
    XML += "</response>";

    return XML;
}

void handleWebsite() {
    server.send_P(200, "text/html", webSiteHtml);
}

void handleXML() {
    server.send(200, "text/xml", generateXML());
}

void sendRedirect() {
    server.send_P(200, "text/html", refreshHtml);
}

void webserver_setup() {
    Serial.println("Setting up webserver...");

    server.on("/", handleWebsite);
    server.on("/xml", handleXML);

    // start the webserver
    server.begin();
}

void webserver_loop() {
    server.handleClient();
}