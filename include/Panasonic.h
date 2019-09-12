//
// Created by naya.vu on 11.09.2019.
//

#ifndef REMOTE_CONTROL_PANASONIC_H
#define REMOTE_CONTROL_PANASONIC_H

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ArduinoJson.h>
#include <AirConditioner.h>

#define PANASONIC_AC_HDR_MARK     3572
#define PANASONIC_AC_HDR_SPACE    1650
#define PANASONIC_AC_BIT_MARK     433
#define PANASONIC_AC_ZERO_SPACE   432
#define PANASONIC_AC_ONE_SPACE    1243
#define PANASONIC_AC_END_MARK     434
#define PANASONIC_AC_END_SPACE    9920

class Panasonic : public AirConditioner {
private:
    const byte header[8]  = { 0x40, 0x04, 0x07, 0x20, 0x00, 0x00, 0x00, 0x60 };
    byte data[18]         = { 0x40, 0x04, 0x07, 0x20, 0x00,
                              0x00, // mode & switch
                              0x00, // temperature
                              0x01,
                              0x00, // fan & swing
                              0x00, 0x00, 0x70, 0x07, 0x00, 0x00, 0x91, 0x00, 0x00 };

    const byte byteLookup[16] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };

    IRsend* _irsend;
    byte reverse(byte n);
    void sendByte(byte n);

public:
    Panasonic(IRsend *irsend) : _irsend(irsend) {};

    void sendState();
};


#endif //REMOTE_CONTROL_PANASONIC_H
