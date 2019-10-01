//
// Created by naya.vu on 29.09.2019.
//

#include <Arduino.h>
#include <FS.h>
#include "StructPersister.h"


uint8_t StructPersister::calculateChecksum(uint8_t* config, size_t len) {
    byte sum = 31;
    for (byte i = 0; i < len; i++) {
        sum ^= config[i];
    }
    return sum;
}

bool StructPersister::load(const char* filename, void* obj, size_t objLen) {
    File f = SPIFFS.open(filename, "r");
    if (!f) {
        return false;
    }
    uint8_t buf[objLen + 1];
    if (f.read(buf, sizeof(buf)) != sizeof(buf)) {
        f.close();
        return false;
    }
    if (calculateChecksum(buf, objLen) != buf[objLen]) {
        f.close();
        return false;
    }
    memcpy(obj, buf, objLen);

    f.close();
    return true;
}

bool StructPersister::persist(const char* filename, void* obj, size_t objLen) {
    uint8_t checksum = calculateChecksum((uint8_t*) obj, objLen);
    File f = SPIFFS.open(filename, "w");
    if (f.write((uint8_t*) obj, objLen) != objLen) {
        f.close();
        return false;
    }
    if (f.write(checksum) != 1) {
        f.close();
        return false;
    }
    f.close();
    return true;
}