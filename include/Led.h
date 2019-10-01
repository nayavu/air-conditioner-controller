//
// Created by naya.vu on 08.09.2019.
//

#ifndef REMOTE_CONTROL_LED_H
#define REMOTE_CONTROL_LED_H

#include <Arduino.h>
#include "App.h"

#define LED_BLINK_INTERVAL 200
#define LED_MODE_CONFIG          0b01010101
#define LED_MODE_CONNECTING_WIFI 0b01010100
#define LED_MODE_CONNECTING_MQTT 0b01010000

class Led {
private:
    AppState *appState;
    byte infoLedPin;
    byte stateLedPin;

    AppState lastAppState = NORMAL;
    byte stateBit;

    unsigned long nextStateLedChange = 0;
    bool nextStateLedChangeOverflow;

    unsigned long nextInfoLedChange = 0;
    bool nextInfoLedChangeOverflow;
    bool infoLedOn;
    byte infoLedBlinkCount;
public:
    Led(byte infoLedPin, byte stateLedPin, AppState *appState);

    void setup();
    void loop();

    void blink(int count = 1);
    void error();
};


#endif //REMOTE_CONTROL_LED_H
