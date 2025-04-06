#include "hardware/IOManager.h"

IOManager::IOManager()
    : detailedStatusEnabled(false) {
    // Initialize LED patterns to OFF
    ledPatterns[0] = LEDPattern::LED_OFF; // ONBOARD_LED
    ledPatterns[1] = LEDPattern::LED_OFF; // ERROR_LED
    ledPatterns[2] = LEDPattern::LED_OFF; // STATUS_LED
    ledPatterns[3] = LEDPattern::LED_OFF; // ROS_LED
    ledPatterns[4] = LEDPattern::LED_OFF; // CMD_LED
}

void IOManager::initialize() {
    // Configure LED pins as outputs
    pinMode(ONBOARD_LED_PIN, OUTPUT);
    pinMode(ERROR_LED, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(ROS_LED_PIN, OUTPUT);
    pinMode(CMD_LED_PIN, OUTPUT);
    
    // Configure switch pins as inputs with pull-up
    pinMode(SW1_PIN, INPUT_PULLUP);
    pinMode(SW2_PIN, INPUT_PULLUP);
    
    // Initialize all LEDs to OFF
    setLED(ONBOARD_LED_PIN, LOW);
    setLED(ERROR_LED, LOW);
    setLED(STATUS_LED_PIN, LOW);
    setLED(ROS_LED_PIN, LOW);
    setLED(CMD_LED_PIN, LOW);
}

void IOManager::playStartupSequence() {
    // Turn on all LEDs one by one
    setLED(ONBOARD_LED_PIN, HIGH);
    delay(100);
    setLED(ERROR_LED, HIGH);
    delay(100);
    setLED(STATUS_LED_PIN, HIGH);
    delay(100);
    setLED(ROS_LED_PIN, HIGH);
    delay(100);
    setLED(CMD_LED_PIN, HIGH);
    delay(100);
    
    // Hold all LEDs on
    delay(500);
    
    // Turn off all LEDs
    setLED(ONBOARD_LED_PIN, LOW);
    setLED(ERROR_LED, LOW);
    setLED(STATUS_LED_PIN, LOW);
    setLED(ROS_LED_PIN, LOW);
    setLED(CMD_LED_PIN, LOW);
}

bool IOManager::isDebugModeEnabled() const {
    // SW1 - Debug mode (inverted due to INPUT_PULLUP)
    return !digitalRead(SW1_PIN);
}

bool IOManager::isErrorResetActive() const {
    // SW2 - Error reset/operational interlock (inverted due to INPUT_PULLUP)
    return !digitalRead(SW2_PIN);
}

void IOManager::setLEDPattern(uint8_t led, LEDPattern pattern) {
    // Map LED pin to index
    int index = -1;
    if (led == ONBOARD_LED_PIN) index = 0;
    else if (led == ERROR_LED) index = 1;
    else if (led == STATUS_LED_PIN) index = 2;
    else if (led == ROS_LED_PIN) index = 3;
    else if (led == CMD_LED_PIN) index = 4;
    
    // Set pattern if valid LED
    if (index >= 0) {
        ledPatterns[index] = pattern;
    }
}

void IOManager::updateLEDs(SystemState state, uint32_t errors, bool rosConnected, bool commandActive) {
    uint32_t currentTime = millis();
    
    // Update patterns based on system state
    
    // ONBOARD_LED: Heartbeat
    setLEDPattern(ONBOARD_LED_PIN, LEDPattern::LED_HEARTBEAT);
    
    // ERROR_LED: Error status
    if (errors) {
        if (errors & ERR_CRITICAL_MASK) {
            setLEDPattern(ERROR_LED, LEDPattern::LED_ON);
        } else {
            setLEDPattern(ERROR_LED, LEDPattern::LED_SLOW_BLINK);
        }
    } else {
        setLEDPattern(ERROR_LED, LEDPattern::LED_OFF);
    }
    
    // STATUS_LED: System state
    switch (state) {
        case SystemState::STATE_INITIALIZING:
            setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_SLOW_BLINK);
            break;
        case SystemState::STATE_CALIBRATING:
            setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_FAST_BLINK);
            break;
        case SystemState::STATE_STANDBY:
            if (isErrorResetActive()) {
                setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_PULSE);
            } else {
                setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_OFF);
            }
            break;
        case SystemState::STATE_OPERATIONAL:
            setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_ON);
            break;
        case SystemState::STATE_ERROR:
            setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_DOUBLE_BLINK);
            break;
        case SystemState::STATE_EMERGENCY_STOP:
            setLEDPattern(STATUS_LED_PIN, LEDPattern::LED_FAST_BLINK);
            break;
    }
    
    // ROS_LED: Connection status
    if (rosConnected) {
        setLEDPattern(ROS_LED_PIN, LEDPattern::LED_ON);
    } else {
        setLEDPattern(ROS_LED_PIN, LEDPattern::LED_SLOW_BLINK);
    }
    
    // CMD_LED: Command activity
    if (commandActive) {
        setLEDPattern(CMD_LED_PIN, LEDPattern::LED_ON);
    } else {
        setLEDPattern(CMD_LED_PIN, LEDPattern::LED_OFF);
    }
    
    // Apply patterns to each LED
    updateLEDWithPattern(ONBOARD_LED_PIN, ledPatterns[0], currentTime);
    updateLEDWithPattern(ERROR_LED, ledPatterns[1], currentTime);
    updateLEDWithPattern(STATUS_LED_PIN, ledPatterns[2], currentTime);
    updateLEDWithPattern(ROS_LED_PIN, ledPatterns[3], currentTime);
    updateLEDWithPattern(CMD_LED_PIN, ledPatterns[4], currentTime);
}

void IOManager::toggleLED(uint8_t led) {
    digitalWrite(led, !digitalRead(led));
}

void IOManager::setLED(uint8_t led, bool state) {
    digitalWrite(led, state ? HIGH : LOW);
}

void IOManager::setDetailedStatusDisplay(bool enabled) {
    detailedStatusEnabled = enabled;
}

void IOManager::updateLEDWithPattern(uint8_t led, LEDPattern pattern, uint32_t currentTime) {
    switch (pattern) {
        case LEDPattern::LED_OFF:
            setLED(led, LOW);
            break;
            
        case LEDPattern::LED_ON:
            setLED(led, HIGH);
            break;
            
        case LEDPattern::LED_SLOW_BLINK:
            // 1Hz blink (500ms on, 500ms off)
            setLED(led, (currentTime % 1000) < 500);
            break;
            
        case LEDPattern::LED_FAST_BLINK:
            // 5Hz blink (100ms on, 100ms off)
            setLED(led, (currentTime % 200) < 100);
            break;
            
        case LEDPattern::LED_DOUBLE_BLINK:
            // Double blink pattern (100ms on, 100ms off, 100ms on, 700ms off)
            {
                uint32_t cycle = currentTime % 1000;
                setLED(led, (cycle < 100) || (cycle >= 200 && cycle < 300));
            }
            break;
            
        case LEDPattern::LED_HEARTBEAT:
            // Heartbeat pattern (100ms on, 100ms off, 100ms on, 700ms off)
            {
                uint32_t cycle = currentTime % 1000;
                setLED(led, (cycle < 100) || (cycle >= 150 && cycle < 250));
            }
            break;
            
        case LEDPattern::LED_PULSE:
            // Pulsing effect using PWM-like timing
            {
                uint32_t cycle = currentTime % 1000;
                uint8_t brightness = 0;
                
                if (cycle < 500) {
                    // Ramping up (0-500ms)
                    brightness = map(cycle, 0, 500, 0, 100);
                } else {
                    // Ramping down (500-1000ms)
                    brightness = map(cycle, 500, 1000, 100, 0);
                }
                
                // Simple digital PWM simulation
                setLED(led, (currentTime % 100) < brightness);
            }
            break;
    }
}