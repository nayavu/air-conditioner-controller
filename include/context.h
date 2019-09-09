// Created by naya.vu on 08.09.2019.
//

#ifndef REMOTE_CONTROL_CONTEXT_H
#define REMOTE_CONTROL_CONTEXT_H


////////////////////////////////////////////
// Default device name
////////////////////////////////////////////

// Generate random default device name.
// Default device name is used only during initialization and may be changed by user in future.
// In general, device name contains prefix 'remote-' and 3 alphanumeric characters ([a-z0-9], 36 different characters) and has 36^3 = 46656 different combinations.
// This fits unsigned int (0-65535) type
// We can generate it using current timestamp (__TIME__ macro in GCC) as hours/3 * 3600 + minutes * 60 + seconds, so we'll have 8*3600 + 3600 + 60 = 32460 possible combinations
#ifndef __TIME__
#define __TIME__ "??:??:??"
#endif

#define __DEFAULT_SUF ((__TIME__[0] == '?' ? 0 : __TIME__[0] - '0') * 10 + (__TIME__[1] == '?' ? 0 : __TIME__[1] - '0')) * 1200 \
  + ((__TIME__[3] == '?' ? 0 : __TIME__[3] - '0') * 10 + (__TIME__[4] == '?' ? 0 : __TIME__[4] - '0')) * 60 \
  + (__TIME__[6] == '?' ? 0 : __TIME__[6] - '0') * 10 + (__TIME__[7] == '?' ? 0 : __TIME__[7] - '0')

#define __DEFAULT_SUF1 __DEFAULT_SUF / (35*35)
#define __DEFAULT_SUF2 (__DEFAULT_SUF - __DEFAULT_SUF1*35*35) / 35
#define __DEFAULT_SUF3 __DEFAULT_SUF - __DEFAULT_SUF1*35*35 - __DEFAULT_SUF2 * 35

inline char deviceIdChar(unsigned int val) {
    return val < 10 ? '0' + val : (val > 36 ? 'z' : 'a' + val - 10);
}

const char DEFAULT_SUF[3] = {deviceIdChar(__DEFAULT_SUF1), deviceIdChar(__DEFAULT_SUF2), deviceIdChar(__DEFAULT_SUF3)};

#undef __DEFAULT_SUF
#undef __DEFAULT_SUF1
#undef __DEFAULT_SUF2
#undef __DEFAULT_SUF3



enum AppState { NORMAL, CONFIG, CONNECTING_WIFI, CONNECTED_WIFI, CONNECTING_MQTT, ERROR };

struct Context {
    AppState state = NORMAL;

    char deviceName[11] = {'r', 'e', 'm', 'o', 't', 'e', '-', DEFAULT_SUF[0], DEFAULT_SUF[1], DEFAULT_SUF[2], 0};
    char wifiSsid[33];
    char wifiPass[65];
    char mqttHost[13];
    char mqttPort[6];
    char mqttUser[17];
    char mqttPass[17];

    bool mqttEnabled;
};

#endif //REMOTE_CONTROL_CONTEXT_H
