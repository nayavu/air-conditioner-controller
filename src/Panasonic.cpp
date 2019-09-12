//
// Created by naya.vu on 11.09.2019.
//

#include "Panasonic.h"

byte Panasonic::reverse(byte n) {
    return (byteLookup[n & 0b1111] << 4) | byteLookup[n >> 4];
}

void Panasonic::sendByte(byte b) {
    for (byte i = 0; i < 8; i++) {
        _irsend->mark(PANASONIC_AC_BIT_MARK);
        _irsend->space(b << i & 0b10000000 ? PANASONIC_AC_ONE_SPACE : PANASONIC_AC_ZERO_SPACE);
    }
}

void Panasonic::sendState() {
    // 6 byte - switch & mode
    if (state.power) {
        data[5] = 0b10010000;
    } else {
        data[5] = 0b00010000;
    }
    switch (state.mode) {
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
    data[6] = reverse(state.t) >> 1;

    // 9 byte - swing & fan
    switch (state.swing) {
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
    switch (state.fan) {
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


    _irsend->enableIROut(38);  // 38khz
    _irsend->space(0);
    _irsend->mark(PANASONIC_AC_HDR_MARK);
    _irsend->space(PANASONIC_AC_HDR_SPACE);

    // 8 bytes - header
    for (byte i = 0; i < 8; i++) {
        sendByte(header[i]);
    }

    _irsend->mark(PANASONIC_AC_END_MARK);
    _irsend->space(PANASONIC_AC_END_SPACE);

    _irsend->mark(PANASONIC_AC_HDR_MARK);
    _irsend->space(PANASONIC_AC_HDR_SPACE);

    byte checksum = 0;
    for (byte i = 0; i < 18; i++) {
        sendByte(data[i]);
        checksum = (byte) (reverse(data[i]) + checksum);
    }
    sendByte(reverse(checksum));
    _irsend->mark(PANASONIC_AC_END_MARK);
    _irsend->space(0);
}
