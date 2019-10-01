#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "App.h"
#include "Led.h"
#include "AirConditionerControl.h"
#include "PanasonicRKDAirConditioner.h"
#include "ConfigManager.h"
#include "UiWebServer.h"

#ifdef DEBUG_ESP_PORT
#define DEBUG_SETUP() DEBUG_ESP_PORT.begin(115200)
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ ); DEBUG_ESP_PORT.print("\n")
#else
#define DEBUG_MSG(...)
#define DEBUG_SETUP()
#endif

#define SYS_LED_PIN 12
#define INFO_LED_PIN 13
#define IR_LED_PIN 14

App app;

Led led(INFO_LED_PIN, SYS_LED_PIN, &app.state);

ConfigManager configManager;

UiWebServer server;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
IRsend irsend(IR_LED_PIN);
PanasonicRKDAirConditioner panasonicAirCond(&irsend);
AirConditionerControl airConditioner(&panasonicAirCond);


#define WIFI_AP_SSID "remote-control"
#define WIFI_AP_PASS "remote-control"
#define WIFI_MAX_CONNECT_TIME 60 * 1000 // 1 min

#define OTA_PASS "R3m0t3-C0ntr0l" // yes, it's not secure to store passwords in the source code on Github :)

// ------ Wifi ------

boolean wifiSetApMode;
boolean wifiSetClientMode;
unsigned long wifiConnectAttemptAt;

// handle wifi connection and
void loopWifi() {
    if (app.state == ERROR) {
        wifiSetApMode = false;
        app.state = CONFIG;
    }
    if (app.state == CONFIG) {
        if (!wifiSetApMode) {
            DEBUG_MSG("Setting up wifi AP with ssid=%s pass=%s", WIFI_AP_SSID, WIFI_AP_PASS);
            wifiSetApMode = true;
            WiFi.mode(WIFI_AP);
            WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
            DEBUG_MSG("IP: %s", WiFi.softAPIP().toString().c_str());

        }
        led.loop();
        return;
    }
    if (app.state == CONNECTING_WIFI && !wifiSetClientMode) {
        DEBUG_MSG("Connecting to wifi");
        wifiSetClientMode = true;

        WiFi.mode(WIFI_STA);
        WiFi.begin(configManager.config->wifiSsid, configManager.config->wifiPass);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        wifiConnectAttemptAt = millis();
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (app.state == CONNECTING_WIFI) {
            app.state = CONNECTED_WIFI;
            DEBUG_MSG("Connected as %s", WiFi.localIP().toString().c_str());
            String domainName = String(configManager.config->deviceName) + ".local";
            if (MDNS.begin(domainName)) {
                DEBUG_MSG("Registered mDNS %s", domainName.c_str());
            } else {
                DEBUG_MSG("Could not register mDNS %s", domainName.c_str());
            }
        }
    } else {

        if (app.state == CONNECTING_WIFI) {
            if (WiFi.status() == WL_NO_SSID_AVAIL) {
                app.state = ERROR;
                led.blink(10);
                DEBUG_MSG("No SSID available");
                return;
            }

            if (WiFi.status() == WL_CONNECT_FAILED) {
                app.state = ERROR;
                led.blink(5);
                DEBUG_MSG("Failed to connect to WiFi");
                return;
            }

            unsigned long current = millis();
            if ((current > wifiConnectAttemptAt && current - wifiConnectAttemptAt > WIFI_MAX_CONNECT_TIME)
                    || (current < wifiConnectAttemptAt && UINT32_MAX - wifiConnectAttemptAt + current > WIFI_MAX_CONNECT_TIME)) {
                // could not connect to wifi within WIFI_MAX_CONNECT_TIME
                DEBUG_MSG("Could not connect to wifi within %d ms, falling back to config mode", WIFI_MAX_CONNECT_TIME);
                app.state = ERROR;
            }
        } else {
            app.state = CONNECTING_WIFI;
        }
    }
}

// ------ MQTT ------

bool mqttSetClient;

const char* HELLO_TOPIC = "/devices/hello";

String mqttSetTopic;
String mqttStateTopic;

void mqttPublishState() {
    String output;
    airConditioner.getState(output);
    mqttClient.publish(mqttStateTopic.c_str(), output.c_str());
}
void onMqttMessage(char* topic, const byte* payload, word l) {
    if (!strcmp(topic, mqttSetTopic.c_str())) {
        if (airConditioner.setState(String((const char*) payload))) {
            led.blink();
        } else {
            led.error();
        }
        mqttPublishState();
    }
}

void setupMqtt() {
    int _port = atoi(configManager.config->mqttPort);
    int port = _port == 0 ? 1883 : _port;
    DEBUG_MSG("MQTT host=%s port=%d", configManager.config->mqttHost, port);
    IPAddress ipAddress;
    ipAddress.fromString(configManager.config->mqttHost);
    mqttClient.setServer(ipAddress, port);
    mqttClient.setCallback(&onMqttMessage);

    mqttSetTopic = String("/devices/remote-control/") + configManager.config->deviceName + "/set";
    mqttStateTopic = String("/devices/remote-control/") + configManager.config->deviceName;
}

void loopMqtt() {
    if (app.state == CONNECTED_WIFI) {
        app.state = CONNECTING_MQTT;
    }
    if (app.state == CONNECTING_MQTT) {
        if (!mqttSetClient) {
            mqttSetClient = true;
            DEBUG_MSG("Connecting to MQTT as id=%s, user=%s, pass=%s", configManager.config->deviceName, configManager.config->mqttUser, configManager.config->mqttPass);
            if (configManager.config->mqttUser && strlen(configManager.config->mqttUser) && configManager.config->mqttPass && strlen(configManager.config->mqttPass)) {
                mqttClient.connect(configManager.config->deviceName, configManager.config->mqttUser, configManager.config->mqttPass);
            } else {
                mqttClient.connect(configManager.config->deviceName);
            }
        }

        if (mqttClient.connected()) {
            DEBUG_MSG("Subscribing to topic %s", mqttSetTopic.c_str());
            mqttClient.subscribe(mqttSetTopic.c_str());
            mqttClient.publish(HELLO_TOPIC, configManager.config->deviceName);
            mqttPublishState();

            app.state = NORMAL;
        }
    }
    if (!mqttClient.connected() && app.state == NORMAL) {
        DEBUG_MSG("Lost connection to MQTT server");
        app.state = CONNECTING_MQTT;
    };
    if (app.state == NORMAL) {
        mqttClient.loop();
    }
}

// ------ web server ------

void setupServer() {
    DEBUG_MSG("Setting up web server");

    server.on("/devices/aircond", HTTP_GET, []() {
        DEBUG_MSG("GET /devices/aircond");
        String buf;
        airConditioner.getState(buf);
        server.send(200, "application/json", buf);
        DEBUG_MSG("HTTP 200\n%s\n", buf.c_str());
    });

    server.on("/devices/aircond", HTTP_POST, []() {
        DEBUG_MSG("POST /devices/aircond\n%s\n", server.arg("plain").c_str());
        bool res = airConditioner.setState(server.arg("plain"));
        if (res) {
            led.blink();
            server.send(200);
            DEBUG_MSG("HTTP 200");
        } else {
            led.error();
            server.send(400);
            DEBUG_MSG("HTTP 400");
        }
        if (app.mqttEnabled) {
            mqttPublishState();
        }
    });

    server.on("/config", HTTP_GET, []() {
        DEBUG_MSG("GET /config");
        bool configLoaded = configManager.load();
        if (!configLoaded) {
            server.send(500);
            DEBUG_MSG("HTTP 500 Could not load config file");
            return;
        }
        String buf = String();
        configManager.toJson(buf);
        server.send(200, "application/json", buf);
        DEBUG_MSG("HTTP 200");
    });

    server.on("/config", HTTP_POST, []() {
        DEBUG_MSG("POST /config\n%s\n", server.arg("plain").c_str());
        bool res = configManager.fromJson(server.arg("plain"));
        if (res) {
            led.blink();
            server.send(200);
            DEBUG_MSG("HTTP 200");
            ESP.restart();
        } else {
            led.error();
            server.send(400);
            DEBUG_MSG("HTTP 400");
        }
    });
    
    server.onNotFound([]() {
        DEBUG_MSG("HTTP 404");
        server.send(404);
    });
    DEBUG_MSG("Starting web server");
    server.begin();
}

// ------ OTA ------

void setupOTA() {
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.setPassword(OTA_PASS);
    ArduinoOTA.onStart([]() {
        led.blink(5);
        if (ArduinoOTA.getCommand() == U_FLASH) {
            DEBUG_MSG("Start OTA updating sketch");
        } else { // U_FS
            DEBUG_MSG("Start OTA updating filesystem");
        };
    });
    ArduinoOTA.onEnd([]() {
        led.blink(5);
        DEBUG_MSG("End OTA updating");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        led.blink(1);
        DEBUG_MSG("Progress: %u%%", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        if (error == OTA_AUTH_ERROR) {
            DEBUG_MSG("OTA auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            DEBUG_MSG("OTA begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            DEBUG_MSG("OTA connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            DEBUG_MSG("OTA receive failed");
        } else if (error == OTA_END_ERROR) {
            DEBUG_MSG("OTA end failed");
        }
        led.blink(10);
    });
    ArduinoOTA.begin();
}

void handleOTA() {
    ArduinoOTA.handle();
}

// ------ main ------

void setup() {
    DEBUG_SETUP();
    DEBUG_MSG("\n-------\n");
    led.setup();
    SPIFFS.begin();

    DEBUG_MSG("Loading config");
    if (!configManager.load()) {
        app.state = CONFIG;
    } else {
        app.state = CONNECTING_WIFI;
    }
    DEBUG_MSG("Config: deviceName=%s wifiSsid=%s wifiPass=%s mqttHost=%s mqttPort=%s mqttUser=%s mqttPass=%s",
              configManager.config->deviceName, configManager.config->wifiSsid, configManager.config->wifiPass, configManager.config->mqttHost, configManager.config->mqttPort, configManager.config->mqttUser, configManager.config->mqttPass);

    DEBUG_MSG("State: %s", app.state == CONFIG ? "CONFIG" : "CONNECTING_WIFI");

    if (app.mqttEnabled) {
        setupMqtt();
    }

    setupServer();

    pinMode(IR_LED_PIN, OUTPUT);
    irsend.begin();

    setupOTA();
}

void loop() {
    handleOTA();

    loopWifi();

    if (app.mqttEnabled) {
        loopMqtt();
    }

    server.handleClient();

    led.loop();
    delay(1);
}
