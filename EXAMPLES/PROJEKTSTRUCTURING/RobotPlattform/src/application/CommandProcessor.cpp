#include "application/CommandProcessor.h"
#include "application/DriveController.h"
#include "hardware/RoboClawDriver.h"
#include "hardware/IMUDriver.h"
#include "system/StateMachine.h"
#include "hardware/IOManager.h"
#include "system/DebugUtils.h"


CommandProcessor::CommandProcessor(DriveController* driveController, 
                                 RoboClawDriver* roboClawDriver,
                                 IMUDriver* imuDriver,
                                 StateMachine* stateMachine,
                                 IOManager* ioManager)
    : driveController(driveController),
      roboClawDriver(roboClawDriver),
      imuDriver(imuDriver),
      stateMachine(stateMachine),
      ioManager(ioManager),
      lastCommandTime(0),
      lastCommandValid(false) {
}

void CommandProcessor::initialize() {
    lastCommandTime = millis();
    lastCommandValid = false;
}

void CommandProcessor::handleWheelVelocityCommand(int32_t* CmdVel) {
    
    // Update last command time
    lastCommandTime = millis();
    lastCommandValid = true;
    
    // Process command flags
    if (CmdVel[CmdVelIndex::Cmd] & CMD_FLAG_EMERGENCY_STOP) {
        debugPrint("Emergency stop flag received");
        stateMachine->transition(SystemState::STATE_EMERGENCY_STOP);
        return;
    }
    
    if (CmdVel[CmdVelIndex::Cmd] & CMD_FLAG_RESET_ENCODERS) {
        debugPrint("Reset encoders flag received");
        driveController->resetEncoders();
    }
    
    if (CmdVel[CmdVelIndex::Cmd] & CMD_FLAG_CALIBRATE_IMU) {
        debugPrint("Calibrate IMU flag received");
        stateMachine->transition(SystemState::STATE_CALIBRATING);
        return;
    }
    
    // Handle enable/disable drive
    if (CmdVel[CmdVelIndex::Cmd] & CMD_FLAG_ENABLE_DRIVE) {
        if (stateMachine->getCurrentState() == SystemState::STATE_STANDBY) {
            // Only transition if SW2 is OFF (safe to operate)
            if (!ioManager->isErrorResetActive()) {
                debugPrint("Enable drive flag received - transitioning to OPERATIONAL");
                stateMachine->setOperationalRequested(true);
            } else {
                debugPrint("Enable drive blocked by SW2");
            }
        }
    }
    
    // Process wheel velocities when in operational state
    if (stateMachine->getCurrentState() == SystemState::STATE_OPERATIONAL) {
        // Extract wheel velocities
        int32_t fl = CmdVel[CmdVelIndex::VelFL]; // Front left
        int32_t rl = CmdVel[CmdVelIndex::VelBL]; // Rear left 
        int32_t fr = CmdVel[CmdVelIndex::VelFR]; // Front right
        int32_t rr = CmdVel[CmdVelIndex::VelBR]; // Rear right
        
        // Set wheel velocities with or without acceleration
        if (CmdVel[CmdVelIndex::Accel]> 0) {
            driveController->setWheelVelocitiesWithAccel(fl, rl, fr, rr, CmdVel[CmdVelIndex::Accel]);
        } else {
            driveController->setWheelVelocities(fl, rl, fr, rr);
        }
    }
}

void CommandProcessor::handleRobotCommand(uint32_t command) {
    // Update last command time
    lastCommandTime = millis();
    lastCommandValid = true;
    
    if (ioManager->isDebugModeEnabled()) {
        DEBUG_PRINT("Received robot command: ");
        DEBUG_PRINTLN(command);
    }
    
    switch (command) {
        case CMD_STOP:
            debugPrint("CMD_STOP - Emergency stop");
            stateMachine->transition(SystemState::STATE_EMERGENCY_STOP);
            break;
            
        case CMD_RESET_DRIVE_FAULT:
            debugPrint("CMD_RESET_DRIVE_FAULT - Resetting drive fault");
            // This will be handled by ErrorHandler later
            break;
            
        case CMD_RESET_ENCODER:
            debugPrint("CMD_RESET_ENCODER - Resetting encoders");
            driveController->resetEncoders();
            break;
            
        case CMD_IMU_CALIBRATION:
            debugPrint("CMD_IMU_CALIBRATION - Starting IMU calibration");
            stateMachine->transition(SystemState::STATE_CALIBRATING);
            break;
            
        case CMD_REBOOT:
            debugPrint("CMD_REBOOT - Restarting system");
            // Force microcontroller reboot
            SCB_AIRCR = 0x05FA0004;
            break;
            
        default:
            if (ioManager->isDebugModeEnabled()) {
                DEBUG_PRINT("Unknown command: ");
                DEBUG_PRINTLN(command);
            }
            break;
    }
}

bool CommandProcessor::hasRecentCommand() const {
    return (millis() - lastCommandTime < COMMAND_TIMEOUT_MS) && lastCommandValid;
}

uint32_t CommandProcessor::getTimeSinceLastCommand() const {
    return millis() - lastCommandTime;
}

void CommandProcessor::debugPrint(const char* message) const {
    if (ioManager && ioManager->isDebugModeEnabled()) {
        DEBUG_PRINTLN(message);
    }
}