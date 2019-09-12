// Created by naya.vu on 08.09.2019.
//

#ifndef REMOTE_CONTROL_CONTEXT_H
#define REMOTE_CONTROL_CONTEXT_H

enum AppState { NORMAL, CONFIG, CONNECTING_WIFI, CONNECTED_WIFI, CONNECTING_MQTT, ERROR };

struct Context {
    AppState state = NORMAL;

    char deviceName[33];
    char wifiSsid[33];
    char wifiPass[65];
    char mqttHost[100];
    char mqttPort[6];
    char mqttUser[17];
    char mqttPass[17];

    bool mqttEnabled;
};

#endif //REMOTE_CONTROL_CONTEXT_H
