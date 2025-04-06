#include "system/DebugUtils.h"
#include "system/StateMachine.h"
#include "application/DriveController.h"
#include "hardware/IOManager.h"

StateMachine::StateMachine(DriveController* driveController, IOManager* ioManager)
    : currentState(SystemState::STATE_INITIALIZING),
      driveController(driveController),
      ioManager(ioManager),
      operationalRequested(false),
      stateEntryTime(0) {
}

void StateMachine::initialize() {
    currentState = SystemState::STATE_INITIALIZING;
    stateEntryTime = millis();
    handleEntryActions(currentState);
}

bool StateMachine::transition(SystemState newState) {
    // Check if transition is allowed
    if (!canTransition(newState)) {
        return false;
    }
    
    // Handle exit actions for current state
    SystemState oldState = currentState;
    handleExitActions(oldState);
    
    // Update state
    currentState = newState;
    stateEntryTime = millis();
    
    // Handle entry actions for new state
    handleEntryActions(newState);
    
    // Print transition if debug is enabled
    printStateTransition(oldState, newState);
    
    return true;
}

SystemState StateMachine::getCurrentState() const {
    return currentState;
}

bool StateMachine::canTransition(SystemState targetState) const {
    // Check transitions based on current state
    switch (currentState) {
        case SystemState::STATE_INITIALIZING:
            // Can only transition to STANDBY or ERROR
            return (targetState == SystemState::STATE_STANDBY ||
                    targetState == SystemState::STATE_ERROR);
        
        case SystemState::STATE_CALIBRATING:
            // Can transition to STANDBY or ERROR
            return (targetState == SystemState::STATE_STANDBY ||
                    targetState == SystemState::STATE_ERROR);
        
        case SystemState::STATE_STANDBY:
            // Can transition to OPERATIONAL, CALIBRATING, ERROR, or EMERGENCY_STOP
            return (targetState == SystemState::STATE_OPERATIONAL ||
                    targetState == SystemState::STATE_CALIBRATING ||
                    targetState == SystemState::STATE_ERROR ||
                    targetState == SystemState::STATE_EMERGENCY_STOP);
        
        case SystemState::STATE_OPERATIONAL:
            // Can transition to STANDBY, ERROR, or EMERGENCY_STOP
            return (targetState == SystemState::STATE_STANDBY ||
                    targetState == SystemState::STATE_ERROR ||
                    targetState == SystemState::STATE_EMERGENCY_STOP);
        
        case SystemState::STATE_ERROR:
            // Can transition to STANDBY or EMERGENCY_STOP
            return (targetState == SystemState::STATE_STANDBY ||
                    targetState == SystemState::STATE_EMERGENCY_STOP);
        
        case SystemState::STATE_EMERGENCY_STOP:
            // Can only transition to INITIALIZING (for reset)
            return (targetState == SystemState::STATE_INITIALIZING);
        
        default:
            return false;
    }
}

void StateMachine::update(uint32_t activeErrors, bool errorResetActive) {
    switch (currentState) {
        case SystemState::STATE_INITIALIZING:
            // Transition to STANDBY after initialization if no errors
            if (millis() - stateEntryTime > 2000 && activeErrors == 0) {
                transition(SystemState::STATE_STANDBY);
            } else if (activeErrors != 0) {
                transition(SystemState::STATE_ERROR);
            }
            break;
        
        case SystemState::STATE_CALIBRATING:
            // Handled externally (IMU calibration)
            break;
        
        case SystemState::STATE_STANDBY:
            // Check for errors
            if (activeErrors & ERR_CRITICAL_MASK) {
                transition(SystemState::STATE_ERROR);
            }
            // Check for operational request
            else if (operationalRequested && !errorResetActive) {
                transition(SystemState::STATE_OPERATIONAL);
            }
            break;
        
        case SystemState::STATE_OPERATIONAL:
            // Check for errors
            if (activeErrors & ERR_CRITICAL_MASK) {
                transition(SystemState::STATE_ERROR);
            }
            // Check for command timeout
            else if (activeErrors & ERR_SYS_COMM_TIMEOUT) {
                transition(SystemState::STATE_STANDBY);
            }
            break;
        
        case SystemState::STATE_ERROR:
            // If error reset is active and errors are cleared, go to STANDBY
            if (errorResetActive && (activeErrors & ERR_CRITICAL_MASK) == 0) {
                transition(SystemState::STATE_STANDBY);
            }
            break;
        
        case SystemState::STATE_EMERGENCY_STOP:
            // Emergency stop requires explicit reset
            break;
    }
}

void StateMachine::setOperationalRequested(bool requested) {
    operationalRequested = requested;
}

const char* StateMachine::getStateName(SystemState state) const {
    switch (state) {
        case SystemState::STATE_INITIALIZING:
            return "INITIALIZING";
        case SystemState::STATE_CALIBRATING:
            return "CALIBRATING";
        case SystemState::STATE_STANDBY:
            return "STANDBY";
        case SystemState::STATE_OPERATIONAL:
            return "OPERATIONAL";
        case SystemState::STATE_ERROR:
            return "ERROR";
        case SystemState::STATE_EMERGENCY_STOP:
            return "EMERGENCY_STOP";
        default:
            return "UNKNOWN";
    }
}

void StateMachine::handleEntryActions(SystemState state) {
    switch (state) {
        case SystemState::STATE_INITIALIZING:
            // No specific entry actions
            break;
        
        case SystemState::STATE_CALIBRATING:
            // No specific entry actions
            break;
        
        case SystemState::STATE_STANDBY:
            // Ensure drive is disabled
            if (driveController) {
                driveController->disable();
            }
            break;
        
        case SystemState::STATE_OPERATIONAL:
            // Enable drive
            if (driveController) {
                driveController->enable();
            }
            operationalRequested = false;
            break;
        
        case SystemState::STATE_ERROR:
            // Disable drive
            if (driveController) {
                driveController->disable();
            }
            break;
        
        case SystemState::STATE_EMERGENCY_STOP:
            // Emergency stop all motors
            if (driveController) {
                driveController->emergencyStop();
                driveController->disable();
            }
            break;
    }
}

void StateMachine::handleExitActions(SystemState state) {
    switch (state) {
        case SystemState::STATE_INITIALIZING:
            // No specific exit actions
            break;
        
        case SystemState::STATE_CALIBRATING:
            // No specific exit actions
            break;
        
        case SystemState::STATE_STANDBY:
            // No specific exit actions
            break;
        
        case SystemState::STATE_OPERATIONAL:
            // Disable drive when leaving operational state
            if (driveController) {
                driveController->disable();
            }
            break;
        
        case SystemState::STATE_ERROR:
            // No specific exit actions
            break;
        
        case SystemState::STATE_EMERGENCY_STOP:
            // No specific exit actions
            break;
    }
}

void StateMachine::printStateTransition(SystemState oldState, SystemState newState) const {
    if (ioManager && ioManager->isDebugModeEnabled()) {
        DEBUG_PRINT("State transition: ");
        DEBUG_PRINT(getStateName(oldState));
        DEBUG_PRINT(" -> ");
        DEBUG_PRINTLN(getStateName(newState));
    }
}