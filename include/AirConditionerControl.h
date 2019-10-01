//
// Created by naya.vu on 11.09.2019.
//

#ifndef REMOTE_CONTROL_AIRCONDITIONERCONTROL_H
#define REMOTE_CONTROL_AIRCONDITIONERCONTROL_H

#include <Arduino.h>
#include <AirConditioner.h>

#define JSON_SIZE JSON_OBJECT_SIZE(6) + 60

// Represents the state of Panasonic CS/CU-E-RKD
class AirConditionerControl {
    AirConditioner* _aircond;
    AirConditionerState state;
public:

    AirConditionerControl(AirConditioner* aircond) : _aircond(aircond) { };

    // Set internal air-conditioner state based on input JSON.
    // Returns true if succeed or false if JSON parse failed.
    // JSON example:
    // {
    //  "power": false, // options: true, false
    //  "t": 23, // options: 16-30
    //  "mode": "auto", // options: "auto", "heat", "cool", "dry"
    //  "fan": "auto", // options: "auto", "1", "2", "3", "4", "5"
    //  "swing": "auto", // options: "auto", "1", "2", "3", "4", "5"
    //  "profile": "normal" // options: "normal", "powerful", "quiet"
    // }
    bool setState(const String &json);

    void getState(String &output);

    void setPower(bool power);
    void setTemperature(byte t);
};


#endif //REMOTE_CONTROL_AIRCONDITIONERCONTROL_H
