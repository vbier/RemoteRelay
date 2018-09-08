#ifndef _WEBSERVER_H
#define _WEBSERVER_H 1

#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void sendRedirect();

void webserver_setup();
void webserver_loop();

#endif