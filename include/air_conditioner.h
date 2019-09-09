//
// Created by naya.vu on 10.09.2019.
//

#ifndef REMOTE_CONTROL_AIR_CONDITIONER_H
#define REMOTE_CONTROL_AIR_CONDITIONER_H

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>

enum PanasonicACMode { MODE_AUTO, MODE_HEAT, MODE_COOL, MODE_DRY };
enum PanasonicACFan { FAN_AUTO, FAN_1, FAN_2, FAN_3, FAN_4, FAN_5 };
enum PanasonicACSwing { SWING_AUTO, SWING_1, SWING_2, SWING_3, SWING_4, SWING_5 };
enum PanasonicACProfile { PROFILE_NORMAL, PROFILE_QUIET, PROFILE_BOOST };

#define PANASONIC_AC_HDR_MARK     3572
#define PANASONIC_AC_HDR_SPACE    1650
#define PANASONIC_AC_BIT_MARK     433
#define PANASONIC_AC_ZERO_SPACE   432
#define PANASONIC_AC_ONE_SPACE    1243
#define PANASONIC_AC_END_MARK     434
#define PANASONIC_AC_END_SPACE    9920

#define JSON_SIZE JSON_OBJECT_SIZE(6) + 60

// Represents the state of Panasonic CS/CU-E-RKD
class AirConditioner {
private:
    IRsend *irsend;

    bool on = false;
    byte t = 23;
    PanasonicACMode mode = MODE_AUTO;
    PanasonicACFan fan = FAN_AUTO;
    PanasonicACSwing swing = SWING_AUTO;
    PanasonicACProfile profile = PROFILE_NORMAL;

    const byte header[8]  = { 0x40, 0x04, 0x07, 0x20, 0x00, 0x00, 0x00, 0x60 };
    byte data[18]         = { 0x40, 0x04, 0x07, 0x20, 0x00,
                              0x00, // mode & switch
                              0x00, // temperature
                              0x01,
                              0x00, // fan & swing
                              0x00, 0x00, 0x70, 0x07, 0x00, 0x00, 0x91, 0x00, 0x00 };

    const byte byteLookup[16] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };

    byte reverse(byte n);
    void sendByte(byte b);

    void sendState();

public:
    AirConditioner(IRsend *irsend);

    // Set internal air-conditioner state based on input JSON.
    // Returns true if succeed or false if JSON parse failed.
    // JSON example:
    // {
    //  "on": false, // options: true, false
    //  "t": 23, // options: 16-30
    //  "mode": "auto", // options: "auto", "heat", "cool", "dry"
    //  "fan": "auto", // options: "auto", "1", "2", "3", "4", "5"
    //  "swing": "auto", // options: "auto", "1", "2", "3", "4", "5"
    //  "profile": "normal" // options: "normal", "quiet", "boost"
    // }
    bool setState(const String &json);

    void getState(String &output);

    void setPower(bool on);
    void setTemperature(byte t);
};


#endif //REMOTE_CONTROL_AIR_CONDITIONER_H
