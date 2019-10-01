//
// Created by naya.vu on 10.09.2019.
//

#include <ArduinoJson.h>
#include "AirConditionerControl.h"

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

    AirConditionerFan fan = FAN_AUTO;
    const char *fanStr = doc["fan"];
    if (fanStr) {
        if (!strcmp(fanStr, "auto")) {
            fan = FAN_AUTO;
        } else if (!strcmp(fanStr, "1")) {
            fan = FAN_1;
        } else if (!strcmp(fanStr, "2")) {
            fan = FAN_2;
        } else if (!strcmp(fanStr, "3")) {
            fan = FAN_3;
        } else if (!strcmp(fanStr, "4")) {
            fan = FAN_4;
        } else if (!strcmp(fanStr, "5")) {
            fan = FAN_5;
        } else {
            return false;
        }
    }

    AirConditionerSwing swing = SWING_AUTO;
    const char *swingStr = doc["swing"];
    if (swingStr) {
        if (!strcmp(swingStr, "auto")) {
            swing = SWING_AUTO;
        } else if (!strcmp(swingStr, "1")) {
            swing = SWING_1;
        } else if (!strcmp(swingStr, "2")) {
            swing = SWING_2;
        } else if (!strcmp(swingStr, "3")) {
            swing = SWING_3;
        } else if (!strcmp(swingStr, "4")) {
            swing = SWING_4;
        } else if (!strcmp(swingStr, "5")) {
            swing = SWING_5;
        } else {
            return false;
        }
    }

    AirConditionerProfile profile = PROFILE_NORMAL;
    const char *profileStr = doc["profile"];
    if (profileStr) {
        if (!strcmp(profileStr, "normal")) {
            profile = PROFILE_NORMAL;
        } else if (!strcmp(profileStr, "powerful")) {
            profile = PROFILE_POWERFUL;
        } else if (!strcmp(profileStr, "quiet")) {
            profile = PROFILE_QUIET;
        } else {
            return false;
        }
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
    if (swingStr) {
        state.swing = swing;
    }
    if (fanStr) {
        state.fan = fan;
    }
    if (profileStr) {
        state.profile = profile;
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

    switch (state.fan) {
        case FAN_1:
            doc["fan"] = "1";
            break;
        case FAN_2:
            doc["fan"] = "2";
            break;
        case FAN_3:
            doc["fan"] = "3";
            break;
        case FAN_4:
            doc["fan"] = "4";
            break;
        case FAN_5:
            doc["fan"] = "5";
            break;
        default:
            doc["fan"] = "auto";
    }

    switch (state.swing) {
        case SWING_1:
            doc["swing"] = "1";
            break;
        case SWING_2:
            doc["swing"] = "2";
            break;
        case SWING_3:
            doc["swing"] = "3";
            break;
        case SWING_4:
            doc["swing"] = "4";
            break;
        case SWING_5:
            doc["swing"] = "5";
            break;
        default:
            doc["swing"] = "auto";
    }

    switch (state.profile) {
        case PROFILE_QUIET:
            doc["profile"] = "quiet";
            break;
        case PROFILE_POWERFUL:
            doc["profile"] = "powerful";
            break;
        default:
            doc["profile"] = "normal";
    }

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