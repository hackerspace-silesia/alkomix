#pragma once
#include <ESP8266WebServer.h>

struct WebData {
    int percents[4];
    bool isReady;
};

void handleMainPage(ESP8266WebServer &server, WebData *data);
void handleNotFound(ESP8266WebServer &server, WebData *data);
void handlePostData(ESP8266WebServer &server, WebData *data); 
