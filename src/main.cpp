#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>

#include "context.h"
#include "led.h"
#include "air_conditioner.h"
#include "ui_web_server.h"

#define SYS_LED_PIN 12
#define INFO_LED_PIN 5
#define IR_LED_PIN 4

Context context;

Led led(INFO_LED_PIN, SYS_LED_PIN, &context.state);

// ------ IR ------
IRsend irsend(IR_LED_PIN);
AirConditioner airConditioner(&irsend);

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
    if (!doc["deviceName"] || !doc["wifiSsid"]) {
        return false;
    }
    strncpy(context.deviceName, doc["deviceName"], sizeof(context.deviceName) - 1);
    strncpy(context.wifiSsid, doc["wifiSsid"], sizeof(context.wifiSsid) - 1);
    if (doc["wifiPass"]) {
        strncpy(context.wifiPass, doc["wifiPass"], sizeof(context.wifiPass) - 1);
    }
    if (doc["mqttHost"]) {
        strncpy(context.wifiPass, doc["mqttHost"], sizeof(context.mqttHost) - 1);
    }
    if (doc["mqttPort"]) {
        strncpy(context.wifiPass, doc["mqttPort"], sizeof(context.mqttPort) - 1);
    }
    if (doc["mqttUser"]) {
        strncpy(context.wifiPass, doc["mqttUser"], sizeof(context.mqttUser) - 1);
    }
    if (doc["mqttPass"]) {
        strncpy(context.wifiPass, doc["mqttPass"], sizeof(context.mqttPass) - 1);
    }
    return false;
}

// ------ Wifi ------

#define WIFI_AP_SSID "remote-control"
#define WIFI_AP_PASS "remote-control"
#define WIFI_MAX_CONNECT_TIME 60 * 1000 // 1 min

boolean wifiSetApMode;
boolean wifiSetClientMode;
unsigned long wifiConnectAttemptAt;

void loopWifi() {
    if (context.state == CONFIG || context.state == ERROR) {
        if (!wifiSetApMode) {
            wifiSetApMode = true;
            WiFi.mode(WIFI_AP);
            WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
        }
        led.loop();
        return;
    }
    if (context.state == CONNECTING_WIFI && !wifiSetClientMode) {
        wifiSetClientMode = true;

        WiFi.mode(WIFI_STA);
        WiFi.begin(context.wifiSsid, context.wifiPass);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        wifiConnectAttemptAt = millis();
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (context.state == CONNECTING_WIFI) {
            context.state = CONNECTED_WIFI;
        }
    } else {
        if (context.state == CONNECTING_WIFI) {
            unsigned long current = millis();
            if ((current > wifiConnectAttemptAt && wifiConnectAttemptAt - wifiConnectAttemptAt > WIFI_MAX_CONNECT_TIME)
                    || (current < wifiConnectAttemptAt && UINT32_MAX - wifiConnectAttemptAt + current > WIFI_MAX_CONNECT_TIME)) {
                // could not connect to wifi within WIFI_MAX_CONNECT_TIME
                context.state = ERROR;
            }
        } else {
            context.state = CONNECTING_WIFI;
        }
    }
}

// ------ MQTT ------

WiFiClient wifiClient;
PubSubClient mqttClient;
bool mqttSetClient;

const char* HELLO_TOPIC = "/devices/hello";
const char* SET_TOPIC = "/devices/remote-control/%s/set";
const char* STATE_TOPIC = "/devices/remote-control/%s";

char setTopic[39];
char stateTopic[36];

void mqttPublishState() {
    String output;
    airConditioner.getState(output);
    mqttClient.publish(stateTopic, output.c_str());
}
void onMqttMessage(char* topic, const byte* payload, word l) {
    if (!strcmp(topic, setTopic)) {
        if (airConditioner.setState(String((const char*) payload))) {
            led.blink();
        } else {
            led.error();
        }
        mqttPublishState();
    }
}

void setupMqtt() {
    mqttClient.setServer(context.mqttHost, atoi(context.mqttPort));
    mqttClient.setCallback(&onMqttMessage);

    sprintf(setTopic, SET_TOPIC, context.deviceName);
    sprintf(stateTopic, STATE_TOPIC, context.deviceName);
}

void loopMqtt() {
    if (context.state == CONNECTED_WIFI) {
        context.state = CONNECTING_MQTT;
    }
    if (context.state == CONNECTING_MQTT) {
        if (!mqttSetClient) {
            mqttSetClient = true;
            mqttClient.connect(context.deviceName, context.mqttUser, context.mqttPass);
        }
    }
    if (mqttClient.connected()) {
        mqttClient.subscribe(setTopic);
        mqttClient.publish(HELLO_TOPIC, context.deviceName);
        mqttPublishState();

        if (context.state != NORMAL) {
            context.state = NORMAL;
        }
    } else {
        if (context.state == NORMAL) {
            context.state = CONNECTING_MQTT;
        }
    };
    if (context.state == NORMAL) {
        mqttClient.loop();
    }
}

// ------ web server ------

UiWebServer server;
void setupServer() {
    server.serveConfig("/config", CONFIG_FILE_NAME);

    server.on("/devices/aircond", HTTP_GET, []() {
        String buf;
        airConditioner.getState(buf);
        server.send(200, "application/json", buf);
    });

    server.on("/devices/aircond", HTTP_POST, []() {
        bool res = airConditioner.setState(server.arg("plain"));
        if (res) {
            led.blink();
            server.send(200);
        } else {
            led.error();
            server.send(400);
        }
        if (context.mqttEnabled) {
            mqttPublishState();
        }
    });
}

void loopServer() {
    server.handleClient();
}


// ------ main ------

void setup() {
    led.setup();

    if (!loadConfig()) {
        context.state = CONFIG;
    } else {
        context.state = CONNECTING_WIFI;
    }

    if (context.mqttEnabled) {
        setupMqtt();
    }

    setupServer();

    irsend.begin();
}

void loop() {
    loopWifi();

    if (context.mqttEnabled) {
        loopMqtt();
    }

    loopServer();

    led.loop();
    delay(1);
}
