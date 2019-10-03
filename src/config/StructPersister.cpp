//
// Created by naya.vu on 29.09.2019.
//

#include <Arduino.h>
#include <FS.h>
#include "StructPersister.h"

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ ); DEBUG_ESP_PORT.print("\n")
#else
#define DEBUG_MSG(...)
#endif

byte StructPersister::calculateChecksum(byte* obj, size_t objLen) {
    uint8_t sum = 31;
    for (size_t i = 0; i < objLen; i++) {
        sum ^= obj[i];
    }
    return sum;
}

bool StructPersister::load(const char* filename, byte* obj, size_t objLen) {
    DEBUG_MSG("Loading struct from %s", filename);
    File f = SPIFFS.open(filename, "r");
    if (!f) {
        DEBUG_MSG("No such file");
        return false;
    }
    byte buf[objLen + 1];
    if (f.read(buf, sizeof(buf)) != sizeof(buf)) {
        f.close();
        DEBUG_MSG("Failed to read %d bytes", sizeof(buf));
        return false;
    }
    if (calculateChecksum(buf, objLen) != buf[objLen]) {
        f.close();
        DEBUG_MSG("Checksum verification failed");
        return false;
    }
    memcpy(obj, buf, objLen);

    f.close();
    DEBUG_MSG("Ok");
    return true;
}

bool StructPersister::persist(const char* filename, byte* obj, size_t objLen) {
    DEBUG_MSG("Saving struct to %s", filename);
    byte checksum = calculateChecksum(obj, objLen);
    DEBUG_MSG("Checksum: %d", checksum);
    File f = SPIFFS.open(filename, "w");
    bool res = f.write(obj, objLen) == objLen && f.write(checksum) == 1;
    if (res) {
        DEBUG_MSG("Ok");
    } else {
        DEBUG_MSG("Failed");
    }
    f.close();
    return res;
}