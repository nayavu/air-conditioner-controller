// Created by naya.vu on 08.09.2019.
//

#ifndef REMOTE_CONTROL_APP_H
#define REMOTE_CONTROL_APP_H

enum AppState { NORMAL, CONFIG, CONNECTING_WIFI, CONNECTED_WIFI, CONNECTING_MQTT, ERROR, REBOOTING };

struct App {
    AppState state = NORMAL;
    bool mqttEnabled;
};

#endif //REMOTE_CONTROL_APP_H
