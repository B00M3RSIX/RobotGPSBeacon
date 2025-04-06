// StateMachine.h
#pragma once
#include <Arduino.h>
#include "Types.h"

// Forward declarations
class DriveController;
class IOManager;

class StateMachine {
public:
    StateMachine(DriveController* driveController, IOManager* ioManager);
    
    /**
     * Initialize the state machine
     */
    void initialize();
    
    /**
     * Transition to a new state
     * @param newState State to transition to
     * @return True if transition was successful
     */
    bool transition(SystemState newState);
    
    /**
     * Get the current system state
     * @return Current state
     */
    SystemState getCurrentState() const;
    
    /**
     * Check if the system can transition to a specific state
     * @param targetState State to check
     * @return True if transition is allowed
     */
    bool canTransition(SystemState targetState) const;
    
    /**
     * Update state machine based on current conditions
     * @param activeErrors Current active errors
     * @param errorResetActive Whether error reset switch is active
     */
    void update(uint32_t activeErrors, bool errorResetActive);
    
    /**
     * Set operational request flag
     * @param requested Whether operation is requested
     */
    void setOperationalRequested(bool requested);
    
    /**
     * Get human-readable name for a state
     * @param state State to get name for
     * @return State name string
     */
    const char* getStateName(SystemState state) const;
    
private:
    SystemState currentState;
    DriveController* driveController;
    IOManager* ioManager;
    bool operationalRequested;
    uint32_t stateEntryTime;
    
    /**
     * Handle actions when entering a state
     * @param state State being entered
     */
    void handleEntryActions(SystemState state);
    
    /**
     * Handle actions when exiting a state
     * @param state State being exited
     */
    void handleExitActions(SystemState state);
    
    /**
     * Print state transition to Serial if debug is enabled
     * @param oldState Previous state
     * @param newState New state
     */
    void printStateTransition(SystemState oldState, SystemState newState) const;
};