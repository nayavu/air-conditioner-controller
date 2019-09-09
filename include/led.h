//
// Created by naya.vu on 08.09.2019.
//

#ifndef REMOTE_CONTROL_LED_H
#define REMOTE_CONTROL_LED_H

#include <Arduino.h>
#include "context.h"

#define LED_BLINK_INTERVAL 200
#define LED_MODE_CONFIG 0b01010101
#define LED_MODE_CONNECTING_MQTT 0b0101000
#define LED_MODE_CONNECTING_WIFI 0b0101010

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

    void blink();
    void error();
};


#endif //REMOTE_CONTROL_LED_H
