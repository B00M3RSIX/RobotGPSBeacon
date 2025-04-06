#pragma once
#include <Arduino.h>
#include "Constants.h"
#include "Types.h"
#include <std_msgs/msg/int32_multi_array.h>

// Forward declarations
class DriveController;
class RoboClawDriver;
class IMUDriver;
class StateMachine;
class IOManager;

class CommandProcessor {
public:
    CommandProcessor(DriveController* driveController, 
                    RoboClawDriver* roboClawDriver,
                    IMUDriver* imuDriver,
                    StateMachine* stateMachine,
                    IOManager* ioManager);
    
    /**
     * Initialize the command processor
     */
    void initialize();
    
    /**
     * Handle wheel velocity command from ROS
     * @param msg Wheel velocity message
     */
    void handleWheelVelocityCommand(int32_t* CmdVel);
    
    /**
     * Handle robot command from ROS
     * @param command Command code
     */
    void handleRobotCommand(uint32_t command);
    
    /**
     * Check if recent command has been received
     * @return True if command received within timeout period
     */
    bool hasRecentCommand() const;
    
    /**
     * Get time since last command
     * @return Time in milliseconds since last command
     */
    uint32_t getTimeSinceLastCommand() const;
    
private:
    DriveController* driveController;
    RoboClawDriver* roboClawDriver;
    IMUDriver* imuDriver;
    StateMachine* stateMachine;
    IOManager* ioManager;
    
    uint32_t lastCommandTime;
    bool lastCommandValid;
    
    /**
     * Print command info if debug is enabled
     * @param message Debug message
     */
    void debugPrint(const char* message) const;
};