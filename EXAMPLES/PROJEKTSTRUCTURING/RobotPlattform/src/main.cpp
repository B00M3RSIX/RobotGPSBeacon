#include <Arduino.h>
#include <Wire.h>
#include "system/DebugUtils.h"

// Include hardware drivers
#include "hardware/RoboClawDriver.h"
#include "hardware/IMUDriver.h"
#include "hardware/IOManager.h"

// Include system modules
#include "system/StateMachine.h"
#include "system/ErrorHandler.h"
#include "system/WatchdogManager.h"

// Include application modules
#include "application/DriveController.h"
#include "application/CommandProcessor.h"

// Include communication module
#include "communication/MicroROSInterface.h"

// Include global definitions
#include "Constants.h"
#include "Types.h"

// Global status array
int64_t roboClawStatus[STATUS_ARRAY_SIZE] = {0};

// System components
RoboClawDriver* roboclawDriver;
IMUDriver* imuDriver;
IOManager* ioManager;
ErrorHandler* errorHandler;
StateMachine* stateMachine;
DriveController* driveController;
CommandProcessor* commandProcessor;
MicroROSInterface* microROS;

// Timing
elapsedMicros loopTimeTimer;
elapsedMillis statusUpdateTimer;

/**
 * Update system status array with current data
 */
void updateSystemStatus() {
    // Update RoboClaw status
    roboclawDriver->updateStatusArray(roboClawStatus);
    
    // Update error status
    uint32_t errL, errR;
    roboclawDriver->getErrors(errL, errR);
    errorHandler->updateFromRoboClawErrors(errL, errR);
    
    // Update additional status fields
    roboClawStatus[RobotState] = static_cast<int64_t>(stateMachine->getCurrentState());
    roboClawStatus[ActiveErrors] = errorHandler->getActiveErrors();
    
    // Pack switch states into a single field (bit 0: SW1, bit 1: SW2)
    uint8_t switchStates = 0;
    if (ioManager->isDebugModeEnabled()) switchStates |= 0x01;
    if (ioManager->isErrorResetActive()) switchStates |= 0x02;
    roboClawStatus[SwitchStates] = switchStates;
}

void setup() {
    
    // Initialize hardware components
    ioManager = new IOManager();
    ioManager->initialize();
    ioManager->playStartupSequence();
    DEBUG_PRINTLN("Robot starting...");
    
    roboclawDriver = new RoboClawDriver(&Serial2);
    imuDriver = new IMUDriver();
    
    // Initialize system components
    errorHandler = new ErrorHandler();
    driveController = new DriveController(roboclawDriver, ioManager);
    stateMachine = new StateMachine(driveController, ioManager);
    
    // Initialize application components
    commandProcessor = new CommandProcessor(driveController, roboclawDriver, 
                                           imuDriver, stateMachine, ioManager);
    
    // Initialize communication
    microROS = new MicroROSInterface(commandProcessor, imuDriver, roboclawDriver);
    
    // Initialize components
    if (!roboclawDriver->initialize()) {
        errorHandler->registerError(ERR_SYS_INIT_FAILURE);
        DEBUG_PRINTLN("RoboClaw initialization failed");
    }
    
    if (!imuDriver->initialize()) {
        errorHandler->registerError(ERR_IMU_COMM_FAILURE);
        DEBUG_PRINTLN("IMU initialization failed");
    }
    
    driveController->initialize();
    stateMachine->initialize();
    commandProcessor->initialize();
    microROS->initialize();
    
    DEBUG_PRINTLN("Initialization complete");
}

void loop() {
    // Reset loop timer
    loopTimeTimer = 0;
    
    // Get switch states
    bool debugMode = ioManager->isDebugModeEnabled();
    bool errorResetActive = ioManager->isErrorResetActive();
    
    // Apply debug mode settings
    driveController->setDebugMode(debugMode);
    
    // Check MicroROS connection
    microROS->checkConnection();
    
    // Update state machine
    stateMachine->update(errorHandler->getActiveErrors(), errorResetActive);
    
    // Update drive controller
    driveController->update();
    
    // Process MicroROS messages if connected
    if (microROS->isConnected()) {
        microROS->processMessages();
    }
    
    // Update and publish status at regular intervals
    if (statusUpdateTimer >= STATUS_PUBLISH_INTERVAL_MS) {
        updateSystemStatus();
        
        if (microROS->isConnected()) {
            microROS->publishStatus(roboClawStatus);
            microROS->publishIMU();
            microROS->publishState(
                stateMachine->getCurrentState(),
                errorHandler->getActiveErrors(),
                debugMode,
                errorResetActive
            );
        }
        
        statusUpdateTimer = 0;
    }
    
    // Update LEDs based on system state
    ioManager->updateLEDs(
        stateMachine->getCurrentState(),
        errorHandler->getActiveErrors(),
        microROS->isConnected(),
        commandProcessor->hasRecentCommand()
    );
    
    // Print debug information if enabled
    if (debugMode) {
        errorHandler->printActiveErrors(true);
    }
    
    // Store loop execution time
    roboClawStatus[LoopTime] = loopTimeTimer;
}


