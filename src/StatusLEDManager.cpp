#include "StatusLEDManager.h"
#include <Wire.h>

StatusLEDManager::StatusLEDManager()
    : currentStatus(LED_STATUS_CONNECTING)
    , ledState(false)
    , blinkTimer(0)
{
}

void StatusLEDManager::begin() {
    Wire.begin();
    rgbLED.begin();
    delay(100);
    rgbLED.setColor(0, 100, 0);
    delay(500);
}

void StatusLEDManager::setStatus(BeaconLEDStatus status) {
    if (status != currentStatus) {
        currentStatus = status;
        ledState = true;  // Reset blink state
        blinkTimer = 0;   // Reset timer
    }
}

void StatusLEDManager::update() {
    updateStatusLED();
}

void StatusLEDManager::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    rgbLED.setColor(red, green, blue);
}

void StatusLEDManager::updateStatusLED() {
    switch (currentStatus) {
        case LED_STATUS_CONNECTING:
            // Cyan blinkend
            if (blinkTimer >= BLINK_INTERVAL_CONNECTING) {
                ledState = !ledState;
                blinkTimer = 0;
            }
            
            if (ledState) {
                rgbLED.setColor(LED_STATUS_CONNECTING_R, LED_STATUS_CONNECTING_G, LED_STATUS_CONNECTING_B);
            } else {
                rgbLED.setColor(0, 0, 0);
            }
            break;
            
        case LED_STATUS_CONNECTED_NO_FIX:
            // Gelb blinkend
            if (blinkTimer >= BLINK_INTERVAL_NO_FIX) {
                ledState = !ledState;
                blinkTimer = 0;
            }
            
            if (ledState) {
                rgbLED.setColor(LED_STATUS_CONNECTED_NO_FIX_R, LED_STATUS_CONNECTED_NO_FIX_G, LED_STATUS_CONNECTED_NO_FIX_B);
            } else {
                rgbLED.setColor(0, 0, 0);
            }
            break;
            
        case LED_STATUS_CONNECTED_FIX:
            // Blau statisch
            rgbLED.setColor(LED_STATUS_CONNECTED_FIX_R, LED_STATUS_CONNECTED_FIX_G, LED_STATUS_CONNECTED_FIX_B);
            break;
            
        case LED_STATUS_ERROR:
            // Rot blinkend
            if (blinkTimer >= BLINK_INTERVAL_ERROR) {
                ledState = !ledState;
                blinkTimer = 0;
            }
            
            if (ledState) {
                rgbLED.setColor(LED_STATUS_ERROR_R, LED_STATUS_ERROR_G, LED_STATUS_ERROR_B);
            } else {
                rgbLED.setColor(0, 0, 0);
            }
            break;
    }
}
