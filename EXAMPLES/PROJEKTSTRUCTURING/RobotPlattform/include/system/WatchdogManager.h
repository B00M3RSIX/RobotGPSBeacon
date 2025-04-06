// WatchdogManager.h
#pragma once
#include <Arduino.h>
#include "Constants.h"

// Forward declarations
class ErrorHandler;

class WatchdogManager {
public:
    WatchdogManager(ErrorHandler* errorHandler);
    
    /**
     * Initialize the watchdog system
     * @param timeoutMs Default timeout in milliseconds
     */
    void initialize(uint32_t timeoutMs);
    
    /**
     * Pet the watchdog to prevent timeout
     */
    void pet();
    
    /**
     * Check if watchdog has timed out
     * @return True if timed out
     */
    bool isTimedOut() const;
    
    /**
     * Set watchdog timeout
     * @param timeoutMs Timeout in milliseconds
     */
    void setTimeout(uint32_t timeoutMs);
    
    /**
     * Get current watchdog timeout
     * @return Timeout in milliseconds
     */
    uint32_t getTimeout() const;
    
    /**
     * Enable or disable the watchdog
     * @param enabled Whether to enable the watchdog
     */
    void setEnabled(bool enabled);
    
    /**
     * Check if watchdog is enabled
     * @return True if enabled
     */
    bool isEnabled() const;
    
    /**
     * Update watchdog state (call in main loop)
     */
    void update();
    
private:
    ErrorHandler* errorHandler;
    uint32_t watchdogTimeout;
    uint32_t lastPetTime;
    bool enabled;
    bool timedOut;
};