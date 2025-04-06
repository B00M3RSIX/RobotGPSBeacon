#include "application/DriveController.h"
#include "hardware/RoboClawDriver.h"
#include "hardware/IOManager.h"
#include "system/DebugUtils.h"

DriveController::DriveController(RoboClawDriver* roboClawDriver, IOManager* ioManager)
    : roboClawDriver(roboClawDriver),
      ioManager(ioManager),
      enabled(false),
      debugModeEnabled(false),
      timeoutHandled(false),
      lastCommandTime(0),
      maxSpeedLimit(MAX_MOTOR_SPEED) {
}

void DriveController::initialize() {
    enabled = false;
    lastCommandTime = millis();
    
    // Initial emergency stop to ensure motors are not moving
    emergencyStop();
}

bool DriveController::setWheelVelocities(int32_t fl, int32_t rl, int32_t fr, int32_t rr) {
    if (!enabled) {
        return false;
    }
    
    // Validate and limit velocities
    if (!validateWheelVelocities(fl, rl, fr, rr)) {
        return false;
    }
    
    // Send velocities to RoboClaw
    bool success = roboClawDriver->setWheelVelocities(fl, rl, fr, rr);
    
    // Reset command timeout
    if (success) {
        resetCommandTimeout();
    }
    
    return success;
}

bool DriveController::setWheelVelocitiesWithAccel(int32_t fl, int32_t rl, int32_t fr, int32_t rr, uint32_t accel) {
    if (!enabled) {
        return false;
    }
    
    // Validate and limit velocities
    if (!validateWheelVelocities(fl, rl, fr, rr)) {
        return false;
    }
    
    // Send velocities to RoboClaw with acceleration
    bool success = roboClawDriver->setWheelVelocitiesWithAccel(fl, rl, fr, rr, accel);
    
    // Reset command timeout
    if (success) {
        resetCommandTimeout();
    }
    
    return success;
}

void DriveController::resetCommandTimeout() {
    lastCommandTime = millis();
    timeoutHandled = false;
}

bool DriveController::isCommandTimedOut() const {
    return (millis() - lastCommandTime >= COMMAND_TIMEOUT_MS);
}

bool DriveController::isEnabled() const {
    return enabled;
}

void DriveController::enable() {
    enabled = true;
    
    if (ioManager && ioManager->isDebugModeEnabled()) {
        DEBUG_PRINTLN("Drive system enabled");
    }
}

void DriveController::disable() {
    if (enabled) {
        // Stop motors when disabling
        roboClawDriver->setWheelVelocities(0, 0, 0, 0);
        enabled = false;
        
        if (ioManager && ioManager->isDebugModeEnabled()) {
            DEBUG_PRINTLN("Drive system disabled");
        }
    }
}

void DriveController::emergencyStop() {
    roboClawDriver->emergencyStop();
    enabled = false;
    
    if (ioManager && ioManager->isDebugModeEnabled()) {
        DEBUG_PRINTLN("EMERGENCY STOP");
    }
}

void DriveController::resetEncoders() {
    roboClawDriver->resetEncoders();
    
    if (ioManager && ioManager->isDebugModeEnabled()) {
        DEBUG_PRINTLN("Encoders reset");
    }
}

void DriveController::setDebugMode(bool enabled) {
    debugModeEnabled = enabled;
    
    // In debug mode, reduce max speed for safety
    if (debugModeEnabled) {
        maxSpeedLimit = MAX_MOTOR_SPEED / 2;
    } else {
        maxSpeedLimit = MAX_MOTOR_SPEED;
    }
}

void DriveController::update() {
    // Check for command timeout
    if (enabled && isCommandTimedOut() && !timeoutHandled) {
        // Stop motors on timeout
        roboClawDriver->setWheelVelocities(0, 0, 0, 0);
        timeoutHandled = true;
        
        if (ioManager && ioManager->isDebugModeEnabled()) {
            DEBUG_PRINTLN("Command timeout - motors stopped");
        }
    }
}

bool DriveController::validateWheelVelocities(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr) {
/*    // Check for invalid values
    if (isnan(fl) || isnan(rl) || isnan(fr) || isnan(rr)) {
        return false;
    }
 */   
    // Apply speed limits
    fl = constrain(fl, -maxSpeedLimit, maxSpeedLimit);
    rl = constrain(rl, -maxSpeedLimit, maxSpeedLimit);
    fr = constrain(fr, -maxSpeedLimit, maxSpeedLimit);
    rr = constrain(rr, -maxSpeedLimit, maxSpeedLimit);
    
    return true;
}