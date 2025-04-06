#pragma once
#include <Arduino.h>
#include "config.h"

class HatchManager {
public:
    HatchManager(uint8_t leftPin, uint8_t rightPin);
    void begin();
    void update();
    bool isHatchOpen() const;
    
private:
    uint8_t leftPin;
    uint8_t rightPin;
    bool hatchOpen;
};
