//
// Created by naya.vu on 11.09.2019.
//

#include "PanasonicRKDAirConditioner.h"

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ ); DEBUG_ESP_PORT.print("\n")
#else
#define DEBUG_MSG(...)
#endif

byte PanasonicRKDAirConditioner::reverse(byte n) {
    return (byteLookup[n & 0b1111] << 4) | byteLookup[n >> 4];
}

void PanasonicRKDAirConditioner::sendByte(byte b) {
    for (byte i = 0; i < 8; i++) {
        _irsend->mark(PANASONIC_AC_BIT_MARK);
        _irsend->space(b << i & 0b10000000 ? PANASONIC_AC_ONE_SPACE : PANASONIC_AC_ZERO_SPACE);
    }
}

void PanasonicRKDAirConditioner::sendData(const byte *bytes, byte length) {
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

    for (byte i = 0; i < length; i++) {
        sendByte(bytes[i]);
    }
    _irsend->mark(PANASONIC_AC_END_MARK);
    _irsend->space(0);
}

void PanasonicRKDAirConditioner::changeState(AirConditionerState &state) {

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

    word checksum = 0;
    for (byte i = 0; i < 18; i++) {
        checksum += reverse(data[i]);
    }
    data[18] = reverse(checksum & 0xFF);
    sendData(data, 19);
    DEBUG_MSG("changed state to pwr=%d, t=%d, mode=%d, fan=%d, swing=%d", state.power, state.t, state.mode, state.fan, state.swing);
}

void PanasonicRKDAirConditioner::changeProfile(AirConditionerState &state) {
    // NORMAL = 0, POWERFUL = 1, QUIET = 2
    byte numSwitches = state.profile > lastState.profile ? state.profile - lastState.profile : 3 + state.profile - lastState.profile;
    for (byte i = 0; i < numSwitches; i++) {
        if (i > 0) {
            delay(PANASONIC_COMMAND_INTERVAL);
        }
        sendData(profileCommand, 8);
        DEBUG_MSG("changed profile");
    }
}

void PanasonicRKDAirConditioner::doSetState(AirConditionerState &state) {
    if (!state.power) {
        DEBUG_MSG("powering off");
        // just turn it off
        changeState(state);
        // and simulate profile reset
        state.profile = PROFILE_NORMAL;
        return;
    }

    bool profileChanged = lastState.profile != state.profile;
    if (profileChanged) {
        DEBUG_MSG("changing profile from %d to %d", lastState.profile, state.profile);
        if (!stateChanged) {
            // switch on and off to reset the profile, then change the profile
            state.power = false;
            changeState(state);
            delay(PANASONIC_COMMAND_INTERVAL);

            state.power = true;
            changeState(state);
            delay(PANASONIC_COMMAND_INTERVAL);
        }
        changeProfile(state);
        if (!stateChanged) {
            return;
        }
    }
    lastState.profile = state.profile;

    if (!memcmp(&lastState, &state, sizeof(state))) {
        DEBUG_MSG("AC state is not changed");
        return;
    }
    if (profileChanged) {
        delay(PANASONIC_COMMAND_INTERVAL);
    }
    changeState(state);
}

bool PanasonicRKDAirConditioner::setState(AirConditionerState &state) {
    // Panasonic RKD remote control encodes the state into a byte stream of 18 bytes + 1 byte checksum and sends it to the AC unit.
    // However, power profile is sent completely differently and the remote control sends 8 byte number to switch the profile to the next one.
    // Therefore, there might be different cases:
    // 1. state changed, profile remained the same - just send the state
    // 2. state remained, profile changed - calculate the number of switches based on the current profile and send the relevant number of switch commands
    //   with a small duration between the commands.
    // 3. both changed - change the profile and then change the state.
    //
    // The second case should consider the rare situation when the remote control is rebooted when AC unit is working and we do not know what was the previous profile.
    // In this case, turn off and on the AC unit to reset the profile (using the passed new state for all other parameters) and then send the required number of profile change commands.
    // Having that AC unit needs some time to process the command, all command sequences should be sent with some interval between them (0.5s - 1s is enough).
    // As a result, in the worst case when the remote control is rebooted and looses its state, and a user wants to change the power profile from some unknown value to QUIET, there will be the following sequence of commands:
    // - switch the AC unit off (using the new state)
    // - delay 500 ms
    // - switch the AC unit on
    // - delay 500 ms
    // - send change profile command (NORMAL -> POWERFUL)
    // - delay 500 ms
    // - send change profile command (POWERFUL -> QUIET)
    // Having that `setState` command might take some significant time, it should be somehow `synchronized` to omit race conditions.

    DEBUG_MSG("Setting AC state...");

    // The simplest way to get rid of race conditions. It's not 100% reliable, since the operations with bool flag are not atomic, but that's enough for our use case.
    if (setStateInProgress) {
        DEBUG_MSG("...cancelled, already in progress");
        return false;
    }
    if (!memcmp(&lastState, &state, sizeof(state))) {
        DEBUG_MSG("...cancelled, nothing changed");
        return false;
    }
    setStateInProgress = true;
    doSetState(state);
    memcpy(&lastState, &state, sizeof(state));
    stateChanged = true;
    setStateInProgress = false;
    return true;
}
