//
// Created by naya.vu on 10.09.2019.
//

#include <ArduinoJson.h>
#include "air_conditioner.h"

AirConditioner::AirConditioner(IRsend *irsend) {
    this->irsend = irsend;
}

byte AirConditioner::reverse(byte n) {
    return (byteLookup[n & 0b1111] << 4) | byteLookup[n >> 4];
}

void AirConditioner::sendByte(byte b) {
    for (byte i = 0; i < 8; i++) {
        irsend->mark(PANASONIC_AC_BIT_MARK);
        irsend->space(b << i & 0b10000000 ? PANASONIC_AC_ONE_SPACE : PANASONIC_AC_ZERO_SPACE);
    }
}

void AirConditioner::sendState() {
    // 6 byte - switch & mode
    if (on) {
        data[5] = 0b10010000;
    } else {
        data[5] = 0b00010000;
    }
    switch (mode) {
        case MODE_HEAT:
            data[5] |= 0b0010;
            break;

        case MODE_COOL:
            data[5] |= 0b1100;
            break;

        case MODE_DRY:
            data[5] |= 0b0100;
            break;

        case MODE_AUTO:
        default:
            data[5] |= 0b0000;
            break;
    }

    // 7 byte - temperature
    // temperature comes in format 0XXXXX00, where XXXXX is decimal temperature value in reversed bit order
    data[6] = reverse(t) >> 1;

    // 9 byte - swing & fan
    switch (swing) {
        case SWING_1:
            data[8] = 0b10000000;
            break;

        case SWING_2:
            data[8] = 0b01000000;
            break;

        case SWING_3:
            data[8] = 0b11000000;
            break;

        case SWING_4:
            data[8] = 0b00100000;
            break;

        case SWING_5:
            data[8] = 0b10100000;
            break;

        case SWING_AUTO:
        default:
            data[8] = 0b11110000;
            break;
    }
    switch (fan) {
        case FAN_1:
            data[8] |= 0b1100;
            break;

        case FAN_2:
            data[8] |= 0b0010;
            break;

        case FAN_3:
            data[8] |= 0b1010;
            break;

        case FAN_4:
            data[8] |= 0b0110;
            break;

        case FAN_5:
            data[8] |= 0b1110;
            break;

        case FAN_AUTO:
        default:
            data[8] |= 0b0101;
            break;
    }


    irsend->enableIROut(38);  // 38khz
    irsend->space(0);
    irsend->mark(PANASONIC_AC_HDR_MARK);
    irsend->space(PANASONIC_AC_HDR_SPACE);

    // 8 bytes - header
    for (byte i = 0; i < 8; i++) {
        sendByte(header[i]);
    }

    irsend->mark(PANASONIC_AC_END_MARK);
    irsend->space(PANASONIC_AC_END_SPACE);

    irsend->mark(PANASONIC_AC_HDR_MARK);
    irsend->space(PANASONIC_AC_HDR_SPACE);

    byte checksum = 0;
    for (byte i = 0; i < 18; i++) {
        sendByte(data[i]);
        checksum = (byte) (reverse(data[i]) + checksum);
    }
    sendByte(reverse(checksum));
    irsend->mark(PANASONIC_AC_END_MARK);
    irsend->space(0);
}

bool AirConditioner::setState(const String &json) {
    DynamicJsonDocument doc(JSON_SIZE);
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    byte t;
    const char *tStr = doc["t"];
    if (tStr) {
        if (t < 16 || t > 30) {
            return false;
        }
        t = atoi(tStr);
    }

    PanasonicACMode mode;
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

    PanasonicACFan fan;
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

    PanasonicACSwing swing;
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

    PanasonicACProfile profile;
    const char *profileStr = doc["profile"];
    if (profileStr) {
        if (!strcmp(profileStr, "normal")) {
            profile = PROFILE_NORMAL;
        } else if (!strcmp(profileStr, "quiet")) {
            profile = PROFILE_QUIET;
        } else if (!strcmp(profileStr, "boost")) {
            profile = PROFILE_BOOST;
        } else {
            return false;
        }
    }

    if (doc.containsKey("on")) {
        this->on = doc["on"];
    }
    if (tStr) {
        this->t = t;
    }
    if (modeStr) {
        this->mode = mode;
    }
    if (swingStr) {
        this->swing = swing;
    }
    if (fanStr) {
        this->fan = fan;
    }
    if (profileStr) {
        this->profile = profile;
    }
    sendState();
    return true;
}

void AirConditioner::getState(String &output) {
    StaticJsonDocument<JSON_SIZE> doc;

    doc["on"] = on;
    doc["t"] = t;

    switch (mode) {
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

    switch (fan) {
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

    switch (swing) {
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

    switch (profile) {
        case PROFILE_QUIET:
            doc["profile"] = "quiet";
            break;
        case PROFILE_BOOST:
            doc["profile"] = "boost";
            break;
        default:
            doc["profile"] = "normal";
    }

    serializeJson(doc, output);
}

void AirConditioner::setPower(bool on) {
    this->on = on;
    sendState();
}

void AirConditioner::setTemperature(byte t) {
    this->t = t;
    sendState();
}