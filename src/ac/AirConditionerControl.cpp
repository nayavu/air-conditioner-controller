//
// Created by naya.vu on 10.09.2019.
//

#include <ArduinoJson.h>
#include "ac/AirConditionerControl.h"

bool AirConditionerControl::setState(const String &json) {
    DynamicJsonDocument doc(JSON_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    byte t = doc["t"];
    if (t && (t < 16 || t > 30)) {
        return false;
    }

    AirConditionerMode mode = MODE_AUTO;
    const char *modeStr = doc["mode"];
    if (modeStr) {
        if (!strcmp(modeStr, "auto")) {
            mode = MODE_AUTO;
        } else if (!strcmp(modeStr, "cool")) {
            mode = MODE_COOL;
        } else if (!strcmp(modeStr, "dry")) {
            mode = MODE_DRY;
        } else if (!strcmp(modeStr, "heat")) {
            mode = MODE_HEAT;
        } else {
            return false;
        }
    }

    byte fan = doc["fan"];
    if (fan > 5) {
        return false;
    }

    byte swing = doc["swing"];
    if (swing > 5) {
        return false;
    }

    byte profile = doc["profile"];
    if (profile > 2) {
        return false;
    }

    if (doc.containsKey("power")) {
        state.power = doc["power"];
    }
    if (t) {
        state.t = t;
    }
    if (modeStr) {
        state.mode = mode;
    }
    if (doc.containsKey("swing")) {
        state.swing = (AirConditionerSwing) swing;
    }
    if (doc.containsKey("fan")) {
        state.fan = (AirConditionerFan) fan;
    }
    if (doc.containsKey("profile")) {
        state.profile = (AirConditionerProfile) profile;
    }
    _aircond->setState(state);
    return true;
}

void AirConditionerControl::getState(String &output) {
    StaticJsonDocument<JSON_SIZE> doc;

    doc["power"] = state.power;
    doc["t"] = state.t;

    switch (state.mode) {
        case MODE_HEAT:
            doc["mode"] = "heat";
            break;
        case MODE_COOL:
            doc["mode"] = "cool";
            break;
        case MODE_DRY:
            doc["mode"] = "dry";
            break;
        default:
            doc["mode"] = "auto";
    }

    doc["fan"] = (byte) state.fan;
    doc["swing"] = (byte) state.swing;
    doc["profile"] = (byte) state.profile;

    serializeJson(doc, output);
}

void AirConditionerControl::setPower(bool power) {
    state.power = power;
    _aircond->setState(state);
}

void AirConditionerControl::setTemperature(byte t) {
    state.t = t;
    _aircond->setState(state);
}