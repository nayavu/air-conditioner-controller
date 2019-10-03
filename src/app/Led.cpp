//
// Created by naya.vu on 08.09.2019.
//

#include "Led.h"

// inverted state, because in my case LEDs are connected to +3.3V
//#define LED_LOW HIGH
//#define LED_HIGH LOW

#define LED_LOW LOW
#define LED_HIGH HIGH

Led::Led(byte infoLedPin, byte sysLedPin, AppState *appState) {
    this->infoLedPin = infoLedPin;
    this->stateLedPin = sysLedPin;
    this->appState = appState;
}

void Led::setup() {
    pinMode(infoLedPin, OUTPUT);
    pinMode(stateLedPin, OUTPUT);
    digitalWrite(infoLedPin, LED_LOW);
    digitalWrite(stateLedPin, LED_LOW);
}

void Led::blink(int count) {
    unsigned long current = millis();
    nextInfoLedChange = current + LED_BLINK_INTERVAL;
    nextInfoLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;
    infoLedOn = LED_HIGH;
    infoLedBlinkCount = count;
    digitalWrite(infoLedPin, infoLedOn);
}

void Led::error() {
    return blink(5);
}

void Led::loop() {
    unsigned long current = millis();
    if (infoLedBlinkCount && nextInfoLedChange && current >= nextInfoLedChange && (!nextInfoLedChangeOverflow || current < UINT32_MAX - LED_BLINK_INTERVAL + nextInfoLedChange)) {
        if (infoLedOn == LED_HIGH) {
            infoLedOn = LED_LOW;
            digitalWrite(infoLedPin, infoLedOn);
            nextInfoLedChange = current + LED_BLINK_INTERVAL;
        } else if (--infoLedBlinkCount > 0) {
            infoLedOn = LED_HIGH;
            digitalWrite(infoLedPin, infoLedOn);
            nextInfoLedChange = current + LED_BLINK_INTERVAL;
        }
    }

    if (lastAppState != *appState) {
        nextStateLedChange = current + LED_BLINK_INTERVAL;
        nextStateLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;
        digitalWrite(stateLedPin, LED_LOW);
        stateBit = 0;

        lastAppState = *appState;

        if (lastAppState == NORMAL) {
            digitalWrite(stateLedPin, LED_LOW);
        }
    }

    if (lastAppState == NORMAL) {
        return;
    }

    if (current >= nextStateLedChange && (!nextStateLedChangeOverflow || current < UINT32_MAX - LED_BLINK_INTERVAL + nextStateLedChange)) {
        nextStateLedChange = current + LED_BLINK_INTERVAL;
        nextStateLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;

        byte mode;
        switch (lastAppState) {
            case CONFIG:
            case ERROR:
                mode = LED_MODE_CONFIG;
                break;
            case CONNECTING_WIFI:
                mode = LED_MODE_CONNECTING_WIFI;
                break;
            case CONNECTING_MQTT:
                mode = LED_MODE_CONNECTING_MQTT;
                break;
            default:
                return;
        }
        digitalWrite(stateLedPin, mode >> stateBit & 1 ? LED_HIGH : LED_LOW);
        if (++stateBit >=8) {
            stateBit = 0;
        }
    }
}