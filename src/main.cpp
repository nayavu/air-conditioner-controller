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

#include "Context.h"
#include "Led.h"
#include "AirConditionerControl.h"
#include "Panasonic.h"
#include "UiWebServer.h"

#define _DEBUG

#ifdef _DEBUG

#define DEBUG_SETUP() Serial.begin(115200)
#define DEBUG_MSG(...) Serial.printf( __VA_ARGS__ ); Serial.print("\n")

#else

#define DEBUG_SETUP()
#define DEBUG_MSG(...)

#endif

#define SYS_LED_PIN 4
#define INFO_LED_PIN 12
#define IR_LED_PIN 5

Context context;

Led led(INFO_LED_PIN, SYS_LED_PIN, &context.state);

UiWebServer server;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
IRsend irsend(IR_LED_PIN);
Panasonic panasonicAirCond(&irsend);
AirConditionerControl airConditioner(&panasonicAirCond);


#define WIFI_AP_SSID "remote-control"
#define WIFI_AP_PASS "remote-control"
#define WIFI_MAX_CONNECT_TIME 60 * 1000 // 1 min

#define OTA_PASS "R3m0t3-C0ntr0l" // yes, it's not secure to store passwords in the source code on Github :)

// ------- CONFIG ------

#define CONFIG_FILE_NAME "/config.json"

bool loadConfig() {
    File f = SPIFFS.open(CONFIG_FILE_NAME, "r");
    if (!f) {
        return false;
    }
    String s = f.readString();
    f.close();

    DynamicJsonDocument doc(JSON_OBJECT_SIZE(7) + 260);
    DeserializationError error = deserializeJson(doc, s);
    if (error) {
        return false;
    }
    const char *deviceName = doc["deviceName"];
    const char *wifiSsid = doc["wifiSsid"];
    const char *wifiPass = doc["wifiPass"];
    const char *mqttHost = doc["mqttHost"];
    const char *mqttPort = doc["mqttPort"];
    const char *mqttUser = doc["mqttUser"];
    const char *mqttPass = doc["mqttPass"];

    if (!deviceName || !wifiSsid) {
        return false;
    }
    strncpy(context.deviceName, deviceName, sizeof(context.deviceName) - 1);
    strncpy(context.wifiSsid, wifiSsid, sizeof(context.wifiSsid) - 1);
    if (wifiPass) {
        strncpy(context.wifiPass, wifiPass, sizeof(context.wifiPass) - 1);
    }
    if (mqttHost) {
        strncpy(context.mqttHost, mqttHost, sizeof(context.mqttHost) - 1);
    }
    if (mqttPort) {
        strncpy(context.mqttPort, mqttPort, sizeof(context.mqttPort) - 1);
    }
    if (mqttUser) {
        strncpy(context.mqttUser, mqttUser, sizeof(context.mqttUser) - 1);
    }
    if (mqttPass) {
        strncpy(context.mqttPass, mqttPass, sizeof(context.mqttPass) - 1);
    }

    context.mqttEnabled = context.mqttHost != 0;
    return true;
}

// ------ Wifi ------

boolean wifiSetApMode;
boolean wifiSetClientMode;
unsigned long wifiConnectAttemptAt;

// handle wifi connection and
void loopWifi() {
    if (context.state == ERROR) {
        wifiSetApMode = false;
        context.state = CONFIG;
    }
    if (context.state == CONFIG) {
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
    if (context.state == CONNECTING_WIFI && !wifiSetClientMode) {
        DEBUG_MSG("Connecting to wifi");
        wifiSetClientMode = true;

        WiFi.mode(WIFI_STA);
        WiFi.begin(context.wifiSsid, context.wifiPass);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        wifiConnectAttemptAt = millis();
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (context.state == CONNECTING_WIFI) {
            context.state = CONNECTED_WIFI;
            DEBUG_MSG("Connected as %s", WiFi.localIP().toString().c_str());
            String domainName = String(context.deviceName) + ".local";
            if (MDNS.begin(domainName)) {
                DEBUG_MSG("Registered mDNS %s", domainName.c_str());
            } else {
                DEBUG_MSG("Could not register mDNS %s", domainName.c_str());
            }
        }
    } else {

        if (context.state == CONNECTING_WIFI) {
            if (WiFi.status() == WL_NO_SSID_AVAIL) {
                context.state = ERROR;
                led.blink(10);
                DEBUG_MSG("No SSID available");
                return;
            }

            if (WiFi.status() == WL_CONNECT_FAILED) {
                context.state = ERROR;
                led.blink(5);
                DEBUG_MSG("Failed to connect to WiFi");
                return;
            }

            unsigned long current = millis();
            if ((current > wifiConnectAttemptAt && current - wifiConnectAttemptAt > WIFI_MAX_CONNECT_TIME)
                    || (current < wifiConnectAttemptAt && UINT32_MAX - wifiConnectAttemptAt + current > WIFI_MAX_CONNECT_TIME)) {
                // could not connect to wifi within WIFI_MAX_CONNECT_TIME
                DEBUG_MSG("Could not connect to wifi within %d ms, falling back to config mode", WIFI_MAX_CONNECT_TIME);
                context.state = ERROR;
            }
        } else {
            context.state = CONNECTING_WIFI;
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
    int _port = atoi(context.mqttPort);
    int port = _port == 0 ? 1883 : _port;
    DEBUG_MSG("MQTT host=%s port=%d", context.mqttHost, port);
    IPAddress ipAddress;
    ipAddress.fromString(context.mqttHost);
    mqttClient.setServer(ipAddress, port);
    mqttClient.setCallback(&onMqttMessage);

    mqttSetTopic = String("/devices/remote-control/") + context.deviceName + "/set";
    mqttStateTopic = String("/devices/remote-control/") + context.deviceName;
}

void loopMqtt() {
    if (context.state == CONNECTED_WIFI) {
        context.state = CONNECTING_MQTT;
    }
    if (context.state == CONNECTING_MQTT) {
        if (!mqttSetClient) {
            mqttSetClient = true;
            DEBUG_MSG("Connecting to MQTT as id=%s, user=%s, pass=%s", context.deviceName, context.mqttUser, context.mqttPass);
            if (context.mqttUser && strlen(context.mqttUser) && context.mqttPass && strlen(context.mqttPass)) {
                mqttClient.connect(context.deviceName, context.mqttUser, context.mqttPass);
            } else {
                mqttClient.connect(context.deviceName);
            }
        }

        if (mqttClient.connected()) {
            DEBUG_MSG("Subscribing to topic %s", mqttSetTopic.c_str());
            mqttClient.subscribe(mqttSetTopic.c_str());
            mqttClient.publish(HELLO_TOPIC, context.deviceName);
            mqttPublishState();

            context.state = NORMAL;
        }
    }
    if (!mqttClient.connected() && context.state == NORMAL) {
        DEBUG_MSG("Lost connection to MQTT server");
        context.state = CONNECTING_MQTT;
    };
    if (context.state == NORMAL) {
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
            loadConfig();
            server.send(200);
            DEBUG_MSG("HTTP 200");
        } else {
            led.error();
            server.send(400);
            DEBUG_MSG("HTTP 400");
        }
        if (context.mqttEnabled) {
            mqttPublishState();
        }
    });

    server.serveUploadableFile("/config", CONFIG_FILE_NAME);
    server.setupAngularApp();

    server.onNotFound([]() {
        DEBUG_MSG("HTTP 404");
        server.send(404);
    });
    DEBUG_MSG("Starting web server");
    server.begin();
}

void loopServer() {
    server.handleClient();
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
    if (!loadConfig()) {
        context.state = CONFIG;
    } else {
        context.state = CONNECTING_WIFI;
    }
    DEBUG_MSG("Config: deviceName=%s wifiSsid=%s wifiPass=%s mqttHost=%s mqttPort=%s mqttUser=%s mqttPass=%s",
            context.deviceName, context.wifiSsid, context.wifiPass, context.mqttHost, context.mqttPort, context.mqttUser, context.mqttPass);

    DEBUG_MSG("State: %s", context.state == CONFIG ? "CONFIG" : "CONNECTING_WIFI");

    if (context.mqttEnabled) {
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

    if (context.mqttEnabled) {
        loopMqtt();
    }

    loopServer();

    led.loop();
    delay(1);
}
