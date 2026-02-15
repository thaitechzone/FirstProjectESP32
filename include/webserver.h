#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// Function Prototypes
void initWebServer();
void handleWebServer();
String getSystemDataJSON();
void displayWiFiInfo(Adafruit_SSD1306 &display);

#endif
