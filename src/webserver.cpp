/**
 * Web Server Implementation for ESP32 PID Controller
 * 
 * Features:
 * - Real-time data display
 * - Temperature chart (SP and PV)
 * - PID parameters display
 * - Relay status monitoring
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DevRelay.h>
#include "webserver.h"
#include "webpage.h"

// WiFi Configuration
const char* ssid = "myHome_2.4GHz";     // WiFi SSID
const char* password = "0939391546";    // WiFi Password

// Async Web Server on port 80
AsyncWebServer server(80);

// External variables from main.cpp
extern double Setpoint, Input, Output;
extern double Kp, Ki, Kd;
extern DevRelay RL1, RL2, RL3;

/**
 * Initialize WiFi in Station Mode and Web Server
 */
void initWebServer() {
  Serial.println(F(""));
  Serial.println(F("=== Initializing Web Server ==="));
  
  // Start WiFi in Station Mode (เชื่อมต่อกับ Router)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(ssid);
  
  // รอการเชื่อมต่อ (timeout 20 วินาที)
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress IP = WiFi.localIP();
    Serial.println(F("WiFi Connected!"));
    Serial.print(F("IP address: "));
    Serial.println(IP);
    Serial.print(F("SSID: "));
    Serial.println(ssid);
    Serial.print(F("Signal Strength: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  } else {
    Serial.println(F("WiFi Connection Failed!"));
    Serial.println(F("Please check SSID and Password"));
    return;
  }
  
  // Configure web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", WEBPAGE);
  });
  
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", getSystemDataJSON());
  });
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "404: Not Found");
  });
  
  // Start server
  server.begin();
  Serial.println(F("Web Server started successfully!"));
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("Open browser and go to: http://"));
    Serial.println(WiFi.localIP());
  }
  Serial.println(F("==============================="));
}

/**
 * Handle incoming web requests
 * Call this in loop() - For async server, this does nothing
 * but kept for compatibility
 */
void handleWebServer() {
  // Async server handles requests automatically
  // No need to call handleClient()
}

/**
 * Display WiFi connection info on OLED
 */
void displayWiFiInfo(Adafruit_SSD1306 &display) {
  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    // Title
    display.setCursor(0, 0);
    display.println(F("WiFi Connected!"));
    
    // SSID
    display.print(F("SSID: "));
    display.println(WiFi.SSID());
    
    // IP Address (แสดงในบรรทัดเดียว)
    display.print(F("IP :"));
    display.println(WiFi.localIP());
    
    // Signal
    display.print(F("Signal: "));
    display.print(WiFi.RSSI());
    display.println(F(" dBm"));
    
    display.display();
  } else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println(F("WiFi"));
    display.setCursor(10, 40);
    display.println(F("FAILED"));
    display.display();
  }
}

/**
 * Get system data as JSON
 */
String getSystemDataJSON() {
  String json = "{";
  
  // Temperature data
  json += "\"setpoint\":" + String(Setpoint, 1) + ",";
  json += "\"pv\":" + String(Input, 1) + ",";
  
  // Output power (0-255 -> 0-100%)
  int outputPercent = map((int)Output, 0, 255, 0, 100);
  json += "\"output\":" + String(outputPercent) + ",";
  
  // PID parameters
  json += "\"kp\":" + String(Kp, 1) + ",";
  json += "\"ki\":" + String(Ki, 2) + ",";
  json += "\"kd\":" + String(Kd, 2) + ",";
  
  // Relay status
  json += "\"relay1\":" + String(RL1.isOn() ? "true" : "false") + ",";
  json += "\"relay2\":" + String(RL2.isOn() ? "true" : "false") + ",";
  json += "\"relay3\":" + String(RL3.isOn() ? "true" : "false");
  
  json += "}";
  
  return json;
}
