//
// Created by naya.vu on 30.09.2019.
//

#ifndef REMOTE_CONTROL_CONFIGMANAGER_H
#define REMOTE_CONTROL_CONFIGMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StructPersister.h"

struct __attribute__((__packed__)) AppConfig {
    char deviceName[33];
    char wifiSsid[33];
    char wifiPass[65];
    char mqttHost[100];
    char mqttPort[6];
    char mqttUser[17];
    char mqttPass[17];
};

class ConfigManager {
private:
    AppConfig _config;
    const char* configFile = "/config.dat";

public:
    const AppConfig* config;
    ConfigManager() : config(&_config) { sprintf(_config.deviceName, "remote-control-%04x", ESP.getChipId()); };

    bool load();
    bool save();

    bool toJson(String& json);
    bool fromJson(const String& json);
};


#endif //REMOTE_CONTROL_CONFIGMANAGER_H
