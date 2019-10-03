//
// Created by naya.vu on 30.09.2019.
//

#include "ConfigManager.h"
#include "FS.h"

bool ConfigManager::isPersisted() {
    return SPIFFS.exists(configFile);
}

bool ConfigManager::load() {
    return StructPersister::load(configFile, (uint8_t*) &_config, sizeof(AppConfig));
}

bool ConfigManager::save() {
    return StructPersister::persist(configFile, (uint8_t*) &_config, sizeof(AppConfig));
}

bool ConfigManager::toJson(String& json) {
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(7) + 260);
    doc["deviceName"] = _config.deviceName;
    doc["wifiSsid"] = _config.wifiSsid;
    doc["wifiPass"] = _config.wifiPass;
    doc["mqttHost"] = _config.mqttHost;
    doc["mqttPort"] = _config.mqttPort;
    doc["mqttUser"] = _config.mqttUser;
    doc["mqttPass"] = _config.mqttPass;
    return serializeJson(doc, json) != 0;
}

bool ConfigManager::fromJson(const String& json) {
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(7) + 260);
    DeserializationError error = deserializeJson(doc, json);
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
    strncpy(_config.deviceName, deviceName, sizeof(_config.deviceName) - 1);
    strncpy(_config.wifiSsid, wifiSsid, sizeof(_config.wifiSsid) - 1);
    if (wifiPass) {
        strncpy(_config.wifiPass, wifiPass, sizeof(_config.wifiPass) - 1);
    }
    if (mqttHost) {
        strncpy(_config.mqttHost, mqttHost, sizeof(_config.mqttHost) - 1);
    }
    if (mqttPort) {
        strncpy(_config.mqttPort, mqttPort, sizeof(_config.mqttPort) - 1);
    }
    if (mqttUser) {
        strncpy(_config.mqttUser, mqttUser, sizeof(_config.mqttUser) - 1);
    }
    if (mqttPass) {
        strncpy(_config.mqttPass, mqttPass, sizeof(_config.mqttPass) - 1);
    }

    return save();
}