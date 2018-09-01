#ifndef _WEBSERVER_H
#define _WEBSERVER_H 1

#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void sendRedirect();
int getRelayFromRequest();

int getRelayIdxFromRequest();
int getDurationFromRequest();

void webserver_setup(ESP8266WebServer::THandlerFunction onHandler, 
                     ESP8266WebServer::THandlerFunction offHandler, 
                     ESP8266WebServer::THandlerFunction stateHandler);
void webserver_loop();

#endif