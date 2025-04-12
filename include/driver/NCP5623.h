#pragma once

#include <Arduino.h>

#define NCP5623_DEFAULT_ADDR 0x39

#define NCP5623_REG_ILED 0x1
#define NCP5623_REG_CHANNEL_BASE 0x2

class NCP5623 {
public:
    NCP5623();
    void begin();
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setRed(uint8_t value);
    void setGreen(uint8_t value);
    void setBlue(uint8_t value);
    void setCurrent(uint8_t iled);
    void mapColors(uint8_t red, uint8_t green, uint8_t blue);
    
private:
    uint8_t _addr;
    uint8_t _red;
    uint8_t _green;
    uint8_t _blue;
    
    void setChannel(uint8_t channel, uint8_t value);
    void writeReg(uint8_t reg, uint8_t value);
};
