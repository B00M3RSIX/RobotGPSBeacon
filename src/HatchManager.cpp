#include "HatchManager.h"

HatchManager::HatchManager(uint8_t leftPin, uint8_t rightPin)
    : leftPin(leftPin)
    , rightPin(rightPin)
    , hatchOpen(false)
{
}

void HatchManager::begin() {
    // Setze Pins als Eingänge mit Pull-down-Widerständen
    pinMode(leftPin, INPUT_PULLDOWN);
    pinMode(rightPin, INPUT_PULLDOWN);
}

void HatchManager::update() {
    // Lese aktuelle Pin-Zustände
    int currentLeftState = digitalRead(leftPin);
    int currentRightState = digitalRead(rightPin);
    
    // Kombiniere die Zustände: HIGH bedeutet offen
    hatchOpen = (currentLeftState == HIGH) || (currentRightState == HIGH);
}

bool HatchManager::isHatchOpen() const {
    return hatchOpen;
}
