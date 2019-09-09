//
// Created by naya.vu on 08.09.2019.
//

#include "led.h"

Led::Led(byte infoLedPin, byte sysLedPin, AppState *appState) {
    this->infoLedPin = infoLedPin;
    this->stateLedPin = sysLedPin;
    this->appState = appState;
}

void Led::setup() {
    pinMode(infoLedPin, OUTPUT);
    pinMode(stateLedPin, OUTPUT);
}

void Led::blink() {
    unsigned long current = millis();
    nextInfoLedChange = current + LED_BLINK_INTERVAL;
    nextInfoLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;
    infoLedOn = HIGH;
    infoLedBlinkCount = 1;
    digitalWrite(infoLedPin, infoLedOn);
}

void Led::error() {
    unsigned long current = millis();
    nextInfoLedChange = current + LED_BLINK_INTERVAL;
    nextInfoLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;
    infoLedOn = HIGH;
    infoLedBlinkCount = 5;
    digitalWrite(infoLedPin, infoLedOn);
}

void Led::loop() {
    unsigned long current = millis();
    if (nextInfoLedChange && current >= nextInfoLedChange && (!nextInfoLedChangeOverflow || current < UINT32_MAX - LED_BLINK_INTERVAL + nextInfoLedChange)) {
        if (--infoLedBlinkCount) {
            nextInfoLedChange = 0;
            digitalWrite(infoLedPin, LOW);
        } else {
            infoLedOn = !infoLedOn;
            digitalWrite(infoLedPin, infoLedOn);
            nextInfoLedChange = current + LED_BLINK_INTERVAL;
        }
    }

    if (lastAppState != *appState) {
        nextStateLedChange = current + LED_BLINK_INTERVAL;
        nextStateLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;
        digitalWrite(stateLedPin, LOW);
        stateBit = 0;

        lastAppState = *appState;

        if (lastAppState == NORMAL) {
            digitalWrite(stateLedPin, 0);
        }
    }

    if (lastAppState == NORMAL) {
        return;
    }

    if (current >= nextStateLedChange && (!nextStateLedChangeOverflow || current < UINT32_MAX - LED_BLINK_INTERVAL + nextStateLedChange)) {
        nextStateLedChange = current + LED_BLINK_INTERVAL;
        nextStateLedChangeOverflow = UINT32_MAX - current <= LED_BLINK_INTERVAL;

        byte mode;
        switch (mode) {
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
        digitalWrite(stateLedPin, mode >> stateBit & 1);
        if (++stateBit >=8) {
            stateBit = 0;
        }
    }
}