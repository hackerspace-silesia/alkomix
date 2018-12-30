#include <Wire.h>
#include <ESP8266WiFi.h> 
#include <DNSServer.h> 
#include <WiFiManager.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266mDNS.h>
#include "webHandler.h"
#include "PumpList.h"

#define PUMPS_LEN 4
#define PERCENT_DIFF 1

char device_name[50];

WiFiManager wifiManager; 
ESP8266WebServer webServer(80);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 lcd(128, 32, &Wire, -1);
bool lcdOk;

// For nodemcu:
// PUMPS
// 5 - D1
// 4 - D2
// 4 - D2
// 13 - D7
// 15 - D8
// LCD
// SCL - D6 - 12
// SDA - D7 - 14

struct WebData webData = {
    .percents = {25, 25, 25, 25},
    .isReady = false
};

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
    Wire.begin(14, 12);
    lcdOk = lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    if(!lcdOk) {
      Serial.println("LCD init failed");
      return;
    }

    lcd.display();
    delay(500);
    lcd.clearDisplay();
    lcd.display();
}

void showTxtWithoutDisplay(String txt) {
    if (!lcdOk) {
        return;
    }
    lcd.clearDisplay();
    lcd.setTextSize(2);
    lcd.setTextColor(WHITE);
    lcd.setCursor(0, 0);
    lcd.println(txt);
}

void showTxt(String txt){
    showTxtWithoutDisplay(txt);
    lcd.display();
}

void showPercent(int index, int percent) {
    if (!lcdOk) {
        return;
    }
    showTxtWithoutDisplay(String("PUMP ") + String(index) + String("\n") + String(percent) + String("%"));
    int width = lcd.width();
    int height = lcd.height();
    int rectHeight = percent * height / 100;
    lcd.fillRect(width - 32, height - rectHeight, width, height, WHITE);
    lcd.display();
}

void showIp() {
  showTxt(String("IP: ") + WiFi.localIP().toString());
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
    showIp();
}

long counterToast = 0;

void loop() {
    webServer.handleClient(); 
    delay(10);

    switch (counterToast) {
      case 0: break;
      case 1: showIp(); counterToast = 0; break;
      default: counterToast--;
    }

    if (!webData.isReady) {
        return;
    }

    pumpList.setSpeed(50);
    pumpList.setCupMilliliter(300);

    int percentSum = 0;
    for(int i=0; i < PUMPS_LEN; i++) {
        int percent = webData.percents[i];
        Serial.printf("Running pump:%d with %d %%...\n", i, percent);

        for(int percentStart=0; percentStart < percent; percentStart += PERCENT_DIFF) {
            int percentDiff = min(PERCENT_DIFF, percent - percentStart);
            percentSum += percentDiff;
            showPercent(i, percentSum);
            pumpList.runPump(i, percentDiff);
        }
    }
    webData.isReady = false;

    counterToast = 300;
    showTxt("TOAST!");
    Serial.println("Done.");
}
