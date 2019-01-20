#include <ArduinoJson.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "Configuration.h"

MDNSResponder mdns;
WiFiClient wifiClient;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
DNSServer dnsServer;

class config: public Configuration {
public:
	char ssid[33];
	char password[33];
	char hostname[17];

	void configure(JsonObject &o);
} cfg;

void config::configure(JsonObject &o) {
	strlcpy(ssid, o[F("ssid")] | "", sizeof(ssid));
	strlcpy(password, o[F("password")] | "", sizeof(password));
	strlcpy(hostname, o[F("hostname")] | "", sizeof(hostname));

	// FIXME: read your configuration parameters here
}

bool connected;
const char *config_file = "/config.json";

void setup() {
	Serial.begin(115200);
	Serial.println(F("Booting!"));

	bool result = SPIFFS.begin();
	if (!result) {
		Serial.print(F("SPIFFS: "));
		Serial.println(result);
		return;
	}

	if (!cfg.read_file(config_file)) {
		Serial.print(F("config!"));
		return;
	}

	WiFi.mode(WIFI_STA);
	WiFi.hostname(cfg.hostname);
	if (*cfg.ssid) {
		WiFi.setAutoReconnect(true);
		WiFi.begin(cfg.ssid, cfg.password);
		for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) {
			delay(500);
			Serial.print('.');
		}
		connected = WiFi.status() == WL_CONNECTED;
	}

	server.on("/config", HTTP_POST, []() {
		if (server.hasArg("plain")) {
			String body = server.arg("plain");
			File f = SPIFFS.open(config_file, "w");
			f.print(body);
			f.close();
			server.send(200);
			ESP.restart();
		} else
			server.send(400, "text/plain", "No body!");
	});
	server.serveStatic("/", SPIFFS, "/index.html");
	server.serveStatic("/config", SPIFFS, config_file);
	server.serveStatic("/js/transparency.min.js", SPIFFS, "/transparency.min.js");

	httpUpdater.setup(&server);
	server.begin();

	if (mdns.begin(cfg.hostname, WiFi.localIP())) {
		Serial.println(F("mDNS started"));
		mdns.addService("http", "tcp", 80);
	} else
		Serial.println(F("Error starting MDNS"));

	if (!connected) {
		WiFi.softAP(cfg.hostname);
		Serial.print(F("Connect to SSID: "));
		Serial.print(cfg.hostname);
		Serial.println(F(" to configure WIFI"));
		dnsServer.start(53, "*", WiFi.softAPIP());
	} else {
		Serial.println();
		Serial.print(F("Connected to "));
		Serial.println(cfg.ssid);
		Serial.println(WiFi.localIP());
	}

	// FIXME: startup
}

void loop() {

	mdns.update();
	server.handleClient();

	if (!connected) {
		dnsServer.processNextRequest();
		return;
	}

	// FIXME: main loop
}
