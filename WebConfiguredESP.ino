#include <ArduinoJson.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "Configuration.h"

MDNSResponder mdns;
WiFiClient wifiClient;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

class config: public Configuration {
public:
  char ssid[33];
  char password[33];
  char hostname[17];

  void configure(JsonObject &o);
} cfg;

void config::configure(JsonObject &o) {
  strncpy_null(ssid, o["ssid"], sizeof(ssid));
  strncpy_null(password, o["password"], sizeof(password));
  strncpy_null(hostname, o["hostname"], sizeof(hostname));
}

bool connected;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Booting!"));
  
  bool result = SPIFFS.begin();
  if (!result) {
    Serial.print(F("SPIFFS: "));
    Serial.println(result);
    return;
  }

  if (!cfg.read_file("/config.json")) {
    Serial.print(F("config!"));
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(cfg.hostname);
  if (*cfg.ssid) {
    WiFi.begin(cfg.ssid, cfg.password);
    for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) {
      delay(500);
      Serial.print(F("."));
    }
    connected = WiFi.status() == WL_CONNECTED;
  }

  server.on("/config", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      File f = SPIFFS.open("/config.json", "w");
      f.print(body);
      f.close();
      ESP.restart();
    } else
      server.send(400, "text/plain", "No body!");
  });
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/config", SPIFFS, "/config.json");
  server.serveStatic("/js/transparency.min.js", SPIFFS, "/transparency.min.js");

  httpUpdater.setup(&server);
  server.begin();

  if (!connected) {
    WiFi.softAP(cfg.hostname);
    Serial.print(F("Connect to SSID: "));
    Serial.print(cfg.hostname);
    Serial.println(F(" and URL http://192.168.4.1 to configure WIFI"));
 
  } else {
    Serial.println();
    Serial.print(F("Connected to "));
    Serial.println(cfg.ssid);
    Serial.println(WiFi.localIP());
  
    if (mdns.begin(cfg.hostname, WiFi.localIP())) {
      Serial.println(F("mDNS started"));
      mdns.addService("http", "tcp", 80);
    } else
      Serial.println(F("Error starting MDNS"));
  }
}

void loop() {

  server.handleClient();
  if (!connected)
    return;

  mdns.update();
}
