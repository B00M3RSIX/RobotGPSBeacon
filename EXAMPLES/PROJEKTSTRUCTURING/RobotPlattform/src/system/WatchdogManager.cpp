#include "system/WatchdogManager.h"
#include "system/ErrorHandler.h"

WatchdogManager::WatchdogManager(ErrorHandler* errorHandler)
    : errorHandler(errorHandler),
      watchdogTimeout(1000), // Default 1 second timeout
      lastPetTime(0),
      enabled(false),
      timedOut(false) {
}

void WatchdogManager::initialize(uint32_t timeoutMs) {
    watchdogTimeout = timeoutMs;
    lastPetTime = millis();
    enabled = true;
    timedOut = false;
}

void WatchdogManager::pet() {
    lastPetTime = millis();
    timedOut = false;
}

bool WatchdogManager::isTimedOut() const {
    return timedOut;
}

void WatchdogManager::setTimeout(uint32_t timeoutMs) {
    watchdogTimeout = timeoutMs;
}

uint32_t WatchdogManager::getTimeout() const {
    return watchdogTimeout;
}

void WatchdogManager::setEnabled(bool enabled) {
    this->enabled = enabled;
    if (enabled) {
        // Reset timer when enabling
        pet();
    }
}

bool WatchdogManager::isEnabled() const {
    return enabled;
}

void WatchdogManager::update() {
    if (!enabled) {
        return;
    }
    
    // Check for timeout
    if ((millis() - lastPetTime) > watchdogTimeout) {
        if (!timedOut) {
            timedOut = true;
            
            // Register watchdog error
            if (errorHandler) {
                errorHandler->registerError(ERR_SYS_WATCHDOG);
            }
            
            // Could also trigger hardware reset here for severe cases
            // SCB_AIRCR = 0x05FA0004; // Force reboot
        }
    }
}