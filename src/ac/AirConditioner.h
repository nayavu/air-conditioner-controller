//
// Created by naya.vu on 10.09.2019.
//

#ifndef REMOTE_CONTROL_AIRCONDITIONER_H
#define REMOTE_CONTROL_AIRCONDITIONER_H

enum AirConditionerMode { MODE_AUTO, MODE_HEAT, MODE_COOL, MODE_DRY };
enum AirConditionerFan { FAN_AUTO, FAN_1, FAN_2, FAN_3, FAN_4, FAN_5 };
enum AirConditionerSwing { SWING_AUTO, SWING_1, SWING_2, SWING_3, SWING_4, SWING_5 };
enum AirConditionerProfile { PROFILE_NORMAL, PROFILE_POWERFUL, PROFILE_QUIET };

struct __attribute__((__packed__)) AirConditionerState {
    bool power = false;
    uint8_t t = 23;
    AirConditionerMode mode = MODE_AUTO;
    AirConditionerFan fan = FAN_AUTO;
    AirConditionerSwing swing = SWING_AUTO;
    AirConditionerProfile profile = PROFILE_NORMAL;
};

// Represents the state of Panasonic CS/CU-E-RKD
class AirConditioner {
public:
    virtual bool setState(AirConditionerState &state);
};


#endif //REMOTE_CONTROL_AIRCONDITIONERCONTROL_H
