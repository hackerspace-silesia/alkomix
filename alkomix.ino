#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <DNSServer.h> 
#include <WiFiManager.h>

#include <ESP8266mDNS.h>
#include "webHandler.h"
#include "PumpList.h"

#define PUMPS_LEN 4

char device_name[50];

WiFiManager wifiManager; 
ESP8266WebServer webServer(80);
struct WebData webData = {
    .percents = {25, 25, 25, 25},
    .isReady = false
};


// For nodemcu:
// 5 - D1
// 4 - D2
// 4 - D2
// 13 - D7
// 15 - D8
int pumpPins[PUMPS_LEN] = {5, 4, 13, 15};
PumpList pumpList(PUMPS_LEN);


void initSerial() {
    Serial.begin(115200);
    Serial.println("Init Alkomix...");
}


void initPumps () {
    Serial.println("Init pump...");
    for(int i=0; i < PUMPS_LEN; i++) {
        int pin = pumpPins[i];
        Pump *pump = new Pump(pin);
        pumpList.addPump(i, pump); 
    }

    bool isOk = pumpList.setupPumps();
    if (!isOk) {
        Serial.println("Something is wrong with pumps :(");
    }
}

void initLCD() {
    Serial.println("Init LCD...");
}

void initWifi() {
    Serial.println("Init Wifi...");
    sprintf(device_name, "Alkomixer-%06X", ESP.getChipId());
    
    Serial.print("Device name: ");
    Serial.println(device_name);
    
    wifiManager.autoConnect(device_name);

    delay(250);
    
    if (!wifiManager.autoConnect(device_name)) {
        Serial.println("failed to connect...");
        delay(1000);
        ESP.reset(); //reset and try again
        delay(5000);
    } else {
        Serial.println("connected to wifi!");
    }

}

void initWeb() {
    webServer.on("/", HTTP_GET,  [](){ handleMainPage(webServer, &webData); });
    webServer.on("/", HTTP_POST, [](){ handlePostData(webServer, &webData); });
    webServer.onNotFound([](){ handleNotFound(webServer, &webData); });
    webServer.begin();

    MDNS.begin("esp8266");
    MDNS.addService("http", "tcp", 80);

    Serial.printf("HTTPServer ready! http://%s.local/\n", device_name);
    delay(300);
}

void setup() {
    initSerial();
    initPumps();
    initLCD();
    initWifi();
    initWeb();
}

void loop() {
    webServer.handleClient(); 
    delay(10);

    if (!webData.isReady) {
        return;
    }

    pumpList.setSpeed(50);
    pumpList.setCupMilliliter(300);
    
    for(int i=0; i < PUMPS_LEN; i++) {
        int percent = webData.percents[i];
        Serial.printf("Running pump:%d with %d %%...\n", i, percent);
        pumpList.runPump(i, percent);
        Serial.println("Done.");
    }
    webData.isReady = false;
}
