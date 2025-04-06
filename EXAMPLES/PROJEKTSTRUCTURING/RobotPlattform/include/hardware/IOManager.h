#pragma once
#include <Arduino.h>
#include "Constants.h"
#include "Types.h"

class IOManager {
public:
    IOManager();
    
    /**
     * Initialize IO pins and configuration
     */
    void initialize();
    
    /**
     * Run startup LED sequence
     */
    void playStartupSequence();
    
    /**
     * Check if debug mode is enabled (SW1)
     * @return True if debug mode is enabled
     */
    bool isDebugModeEnabled() const;
    
    /**
     * Check if error reset is active (SW2)
     * @return True if error reset is active
     */
    bool isErrorResetActive() const;
    
    /**
     * Set an LED to a specific pattern
     * @param led LED pin
     * @param pattern Pattern to set
     */
    void setLEDPattern(uint8_t led, LEDPattern pattern);
    
    /**
     * Update all LEDs based on system state and errors
     * @param state Current system state
     * @param errors Active error bitfield
     * @param rosConnected Whether ROS is connected
     * @param commandActive Whether commands are being received
     */
    void updateLEDs(SystemState state, uint32_t errors, bool rosConnected, bool commandActive);
    
    /**
     * Toggle an LED
     * @param led LED pin
     */
    void toggleLED(uint8_t led);
    
    /**
     * Set an LED state
     * @param led LED pin
     * @param state State to set (HIGH or LOW)
     */
    void setLED(uint8_t led, bool state);
    
    /**
     * Set detailed status display mode
     * @param enabled Whether to enable detailed status display
     */
    void setDetailedStatusDisplay(bool enabled);
    
private:
    bool detailedStatusEnabled;
    LEDPattern ledPatterns[5]; // Patterns for each LED
    
    /**
     * Update a specific LED based on its pattern
     * @param led LED pin
     * @param pattern Pattern to apply
     * @param currentTime Current time in milliseconds
     */
    void updateLEDWithPattern(uint8_t led, LEDPattern pattern, uint32_t currentTime);
};