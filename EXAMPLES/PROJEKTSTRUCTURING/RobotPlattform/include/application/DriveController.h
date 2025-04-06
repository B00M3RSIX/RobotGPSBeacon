// DriveController.h
#pragma once
#include <Arduino.h>
#include "Constants.h"
#include "Types.h"

// Forward declarations
class RoboClawDriver;
class IOManager;

class DriveController {
public:
    DriveController(RoboClawDriver* roboClawDriver, IOManager* ioManager);
    
    /**
     * Initialize the drive controller
     */
    void initialize();
    
    /**
     * Set wheel velocities
     * @param fl Front left wheel velocity
     * @param rl Rear left wheel velocity
     * @param fr Front right wheel velocity
     * @param rr Rear right wheel velocity
     * @return True if command was successful
     */
    bool setWheelVelocities(int32_t fl, int32_t rl, int32_t fr, int32_t rr);
    
    /**
     * Set wheel velocities with acceleration limit
     * @param fl Front left wheel velocity
     * @param rl Rear left wheel velocity
     * @param fr Front right wheel velocity
     * @param rr Rear right wheel velocity
     * @param accel Acceleration limit
     * @return True if command was successful
     */
    bool setWheelVelocitiesWithAccel(int32_t fl, int32_t rl, int32_t fr, int32_t rr, uint32_t accel);
    
    /**
     * Reset last command time
     */
    void resetCommandTimeout();
    
    /**
     * Check if command timeout has occurred
     * @return True if timeout has occurred
     */
    bool isCommandTimedOut() const;
    
    /**
     * Check if drive is enabled
     * @return True if drive is enabled
     */
    bool isEnabled() const;
    
    /**
     * Enable drive
     */
    void enable();
    
    /**
     * Disable drive
     */
    void disable();
    
    /**
     * Emergency stop all motors
     */
    void emergencyStop();
    
    /**
     * Reset all encoders
     */
    void resetEncoders();
    
    /**
     * Set debug mode (reduced speed)
     * @param enabled Whether debug mode is enabled
     */
    void setDebugMode(bool enabled);
    
    /**
     * Update drive controller (handle timeouts, etc.)
     */
    void update();
    
private:
    RoboClawDriver* roboClawDriver;
    IOManager* ioManager;
    
    bool enabled;
    bool debugModeEnabled;
    bool timeoutHandled;
    uint32_t lastCommandTime;
    int32_t maxSpeedLimit;
    
    /**
     * Validate and limit wheel velocities
     * @param fl Front left wheel velocity (input/output)
     * @param rl Rear left wheel velocity (input/output)
     * @param fr Front right wheel velocity (input/output)
     * @param rr Rear right wheel velocity (input/output)
     * @return True if velocities are valid
     */
    bool validateWheelVelocities(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr);
};