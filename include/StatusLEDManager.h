#pragma once
#include <Arduino.h>
#include "driver/NCP5623.h"
#include "config.h"

enum BeaconLEDStatus {
    LED_STATUS_CONNECTING,
    LED_STATUS_CONNECTED_NO_FIX,
    LED_STATUS_CONNECTED_FIX,
    LED_STATUS_ERROR
};

class StatusLEDManager {
public:
    StatusLEDManager();
    void begin();
    void update();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    
    // RGB-LED Methoden
    void setStatus(BeaconLEDStatus status);
    
private:
    NCP5623 rgbLED;
    
    BeaconLEDStatus currentStatus;
    bool ledState;
    elapsedMillis blinkTimer;
    
    void updateStatusLED();
};
