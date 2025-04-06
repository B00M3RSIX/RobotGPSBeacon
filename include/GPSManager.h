#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "config.h"
#include "NavSatFixData.h"

class GPSManager {
public:
    GPSManager();
    void begin(HardwareSerial& serial, unsigned long baud);
    void update();
    bool hasValidFix() const;
    const NavSatFixData& getNavSatFixData() const;
    
private:
    TinyGPSPlus gps;
    NavSatFixData navSatData;
    HardwareSerial* gpsSerial;
    
    void updateNavSatFixData();
    void checkSerialData();
    
    unsigned long last_serial_check;
    uint32_t serial_bytes_received;
    uint32_t last_serial_bytes_count;
};
