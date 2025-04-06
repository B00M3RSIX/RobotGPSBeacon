# Mecabot Microcontroller Architecture - Updated

## 1. Project Overview

The Mecabot is a four-wheel robotic platform that can be configured as either a mecanum drive or skid-steer system. The architecture has been completely refactored to be modular, maintainable, and to move kinematic calculations to the ROS layer.

### Key Features

- **Direct wheel control**: ROS sends direct wheel velocity commands
- **Enhanced safety**: Robust error handling and fault detection
- **State machine**: Clear system states with well-defined transitions
- **Hardware abstraction**: Clean interfaces to physical components
- **Switch functionality**: Debug mode toggle and safety interlock via physical switches
- **Status reporting**: Comprehensive status data for the ROS layer

## 2. Project Structure

```
/
├── include/
│   ├── hardware/
│   │   ├── RoboClawDriver.h      # Motor controller interface
│   │   ├── IMUDriver.h           # IMU sensor interface
│   │   └── IOManager.h           # I/O and LED management
│   │
│   ├── system/
│   │   ├── StateMachine.h        # System state management
│   │   ├── ErrorHandler.h        # Error tracking and handling
│   │   └── WatchdogManager.h     # Watchdog timer management
│   │
│   ├── application/
│   │   ├── DriveController.h     # Drive system management
│   │   └── CommandProcessor.h    # Command processing
│   │
│   ├── communication/
│   │   └── MicroROSInterface.h   # MicroROS communication
│   │
│   ├── Constants.h               # System-wide constants
│   └── Types.h                   # Common type definitions
│
├── src/
│   ├── hardware/
│   │   ├── RoboClawDriver.cpp
│   │   ├── IMUDriver.cpp
│   │   └── IOManager.cpp
│   │
│   ├── system/
│   │   ├── StateMachine.cpp
│   │   ├── ErrorHandler.cpp
│   │   └── WatchdogManager.cpp
│   │
│   ├── application/
│   │   ├── DriveController.cpp
│   │   └── CommandProcessor.cpp
│   │
│   ├── communication/
│   │   └── MicroROSInterface.cpp
│   │
│   └── main.cpp                 # Main program
```

## 3. Architecture Layers

The system is organized into four distinct layers:

### 3.1. Hardware Layer

This layer provides direct interfaces to hardware components:

- **RoboClawDriver**: Interfaces with the RoboClaw motor controllers
  - Direct wheel velocity commands
  - Encoder reading
  - Status and error monitoring
  
- **IMUDriver**: Manages the BNO055 IMU sensor
  - Orientation data
  - Calibration handling
  - EEPROM storage for calibration data
  
- **IOManager**: Handles I/O operations
  - LED status display
  - Switch reading (debug mode, error reset)
  - Visual feedback patterns

### 3.2. System Layer

This layer provides core system functions:

- **StateMachine**: Manages system states
  - States: INITIALIZING, CALIBRATING, STANDBY, OPERATIONAL, ERROR, EMERGENCY_STOP
  - State transitions with entry/exit actions
  - Safety-focused state management
  
- **ErrorHandler**: Tracks and processes errors
  - Structured error codes
  - Error recording and tracking
  - Error mapping from RoboClaw to system errors
  
- **WatchdogManager**: Provides timeout safety
  - Software watchdog timer
  - System reset capability
  - Configurable timeout periods

### 3.3. Application Layer

This layer implements robot-specific functionality:

- **DriveController**: Manages motor control
  - Direct wheel velocity setting
  - Safety limits and validation
  - Command timeout handling
  
- **CommandProcessor**: Processes commands from ROS
  - Wheel velocity command handling
  - Robot command processing
  - Command validation

### 3.4. Communication Layer

This layer handles external communication:

- **MicroROSInterface**: Manages ROS communication
  - Message publishing (status, IMU, state)
  - Command reception
  - Connection management

## 4. System States

The StateMachine manages the following states:

1. **INITIALIZING**: System startup and initialization
2. **CALIBRATING**: IMU calibration in progress
3. **STANDBY**: System ready but motors disabled
4. **OPERATIONAL**: Normal operation with motors enabled
5. **ERROR**: Error condition, motors stopped
6. **EMERGENCY_STOP**: Emergency stop activated, requires reset

State transitions follow strict rules to ensure safety:

- **STANDBY → OPERATIONAL**: Only if SW2 is OFF
- **ERROR → STANDBY**: Only if SW2 is ON and errors cleared
- **Any state → EMERGENCY_STOP**: Always allowed
- **EMERGENCY_STOP → INITIALIZING**: Only via system reset

## 5. Switch Functions

Two hardware switches provide physical control:

- **SW1 - Debug Mode**: 
  - When ON: Enables serial debugging and reduces speed limits (50%)
  - When OFF: Normal operation at full speed
  
- **SW2 - Error Reset/Safety Interlock**: 
  - When ON: Allows clearing errors (transition from ERROR to STANDBY)
  - When OFF: Allows operation (transition from STANDBY to OPERATIONAL)

This combination provides both safety and diagnostic capabilities.

## 6. LED Status Indicators

Five LEDs provide visual status information:

- **ONBOARD_LED**: Heartbeat pattern to indicate system alive
- **ERROR_LED**: Indicates error conditions
  - OFF: No errors
  - Solid: Critical error
  - Slow blink: Warning condition
  
- **STATUS_LED**: Shows system state
  - OFF: STANDBY
  - Solid: OPERATIONAL
  - Slow blink: INITIALIZING
  - Fast blink: CALIBRATING or EMERGENCY_STOP
  - Double blink: ERROR
  
- **ROS_LED**: Shows ROS connection status
  - OFF: Not connected
  - Solid: Connected
  - Slow blink: Attempting connection
  
- **CMD_LED**: Shows command activity
  - OFF: No recent commands
  - Solid: Recent command received

## 7. Status Reporting

The status array has been expanded to include more comprehensive information:

```
roboClawStatus[23] = {
    // Encoders (0-3)
    encFL, encBL, encFR, encBR,
    
    // Speeds (4-7)
    spdFL, spdRL, spdFR, spdRR,
    
    // Currents (8-11)
    curFL, curBL, curFR, curBR,
    
    // Voltages (12-13)
    voltL, voltR,
    
    // Errors (14-15)
    errL, errR,
    
    // Temperatures (16-17)
    tempL, tempR,
    
    // Timing (18-19)
    timestamp, loopTime,
    
    // Additional fields (20-22)
    robotState, activeErrors, switchStates
};
```

## 8. Communication Protocol

### 8.1. Incoming Messages

- **wheel_cmd** (WheelVelocityMsg):
  - `velocities[4]`: Direct wheel velocities (FL, RL, FR, RR)
  - `acceleration`: Optional acceleration limit
  - `command_flags`: Control flags (emergency stop, reset encoders, etc.)

- **robo_cmd** (UInt32):
  - Command codes for system functions (stop, reset, calibrate, etc.)

### 8.2. Outgoing Messages

- **roboclawstatus** (Int64MultiArray):
  - Status array with comprehensive system information
  
- **Imu** (sensor_msgs/Imu):
  - IMU orientation, angular velocity, and acceleration
  
- **state** (UInt8):
  - Current system state
  
- **errors** (UInt32):
  - Active error bitfield

## 9. Implementation Notes

### 9.1. Error Handling

The system uses a structured error handling approach:

- Errors are defined as bitfields in Constants.h
- ErrorHandler tracks active errors and error history
- RoboClaw errors are mapped to system error codes
- Critical errors trigger state transitions

### 9.2. Command Validation

All commands undergo validation before execution:

- Wheel velocities are checked for valid ranges
- Commands are only executed in appropriate states
- SW2 provides a physical interlock for operational state

### 9.3. Timeout Handling

Multiple timeout mechanisms ensure safety:

- Command timeout: Stop motors if no commands received
- MicroROS timeout: Handle ROS connection loss
- Watchdog timeout: Detect system hangs

## 10. Implementation Instructions

To implement this architecture:

1. Create all header files first
2. Implement hardware layer classes
3. Implement system layer classes
4. Implement application layer classes
5. Implement communication layer
6. Integrate in main.cpp

## 11. Testing and Validation

To validate the implementation:

1. Test each module individually
2. Verify state transitions with SW2
3. Test timeout handling
4. Verify error detection and recovery
5. Validate LED status indications
6. Test ROS communication

This modular architecture provides a solid foundation for the Mecabot, with clear separation of concerns and robust error handling. The removal of kinematics from the microcontroller allows for easier switching between drive types (mecanum/skid-steer) at the ROS level.
