#pragma once

// Pin Definitions
#define ONBOARD_LED_PIN 13
#define ERROR_LED 5
#define STATUS_LED_PIN 4
#define ROS_LED_PIN 3
#define CMD_LED_PIN 2
#define SW1_PIN 11
#define SW2_PIN 12

// RoboClaw Configuration
#define ROBOCLAW_ADDRESS_LEFT 0x80
#define ROBOCLAW_ADDRESS_RIGHT 0x81
#define ROBOCLAW_BAUD_RATE 115200
#define ROBOCLAW_TIMEOUT 10000

// Motor Limits
#define MAX_MOTOR_SPEED 2200
#define DEFAULT_ACCELERATION 10000

// Battery Voltage Limits (0.1V)
#define BATTERY_MIN_VOLTAGE 105  // 10.5V
#define BATTERY_MAX_VOLTAGE 129  // 12.9V

// Drive System Parameters
#define WHEEL_RADIUS 0.048          // 96mm diameter / 2
#define WHEEL_SEPARATION_WIDTH 0.416
#define WHEEL_SEPARATION_LENGTH 0.336
#define ENCODER_RATIO ((((1.0+(46.0/17.0))) * (1.0+(46.0/11.0))) * 28.0)

// Timing Constants
#define COMMAND_TIMEOUT_MS 250
#define HEARTBEAT_INTERVAL_MS 500
#define STATUS_PUBLISH_INTERVAL_MS 50
#define IMU_PUBLISH_INTERVAL_MS 100
#define ERROR_CHECK_INTERVAL_MS 100
#define CONNECTION_CHECK_INTERVAL_MS 1000

// Status Array Size and Indices
#define STATUS_ARRAY_SIZE 23

// Status Array Indices
enum StatusIndex {
    EncFL = 0,      // Front left encoder position
    EncBL = 1,      // Back left encoder position
    EncFR = 2,      // Front right encoder position
    EncBR = 3,      // Back right encoder position
    SpdFL = 4,      // Front left speed
    SpdRL = 5,      // Rear left speed
    SpdFR = 6,      // Front right speed
    SpdRR = 7,      // Rear right speed
    CurFL = 8,      // Front left current
    CurBL = 9,      // Back left current
    CurFR = 10,     // Front right current
    CurBR = 11,     // Back right current
    VolL = 12,      // Left voltage
    VolR = 13,      // Right voltage
    ErrL = 14,      // Left error status
    ErrR = 15,      // Right error status
    TempL = 16,     // Left temperature
    TempR = 17,     // Right temperature
    Timestamp = 18, // Timestamp (ms)
    LoopTime = 19,  // Loop execution time (μs)
    RobotState = 20,// Current system state
    ActiveErrors = 21, // Error bitfield
    SwitchStates = 22  // Bit 0: SW1, Bit 1: SW2
};

// Status Array Size and Indices
#define CMD_VEL_ARRAY_SIZE 6

// Status Array Indices
enum CmdVelIndex {
    VelFL = 0,      // Front left Setpoint Velocity
    VelBL = 1,      // Back left Setpoint Velocity
    VelFR = 2,      // Front right Setpoint Velocity
    VelBR = 3,      // Back right Setpoint Velocity
    Accel = 4,      // Acceleration 
    Cmd = 5,        // Robot Command
};

// Command Flags
#define CMD_FLAG_EMERGENCY_STOP  0x01
#define CMD_FLAG_RESET_ENCODERS  0x02
#define CMD_FLAG_CALIBRATE_IMU   0x04
#define CMD_FLAG_ENABLE_DRIVE    0x08

// Robot Commands
#define CMD_STOP                0    // Stop unit and reset
#define CMD_RESET_DRIVE_FAULT   1    // Reset drive fault
#define CMD_RESET_ENCODER       10   // Reset encoder
#define CMD_IMU_CALIBRATION     11   // Perform IMU calibration
#define CMD_REBOOT              666  // Reboot the system

// Error Codes
#define ERR_NONE                 0x00000000
#define ERR_SYS_INIT_FAILURE     0x00000001
#define ERR_SYS_COMM_TIMEOUT     0x00000002
#define ERR_SYS_WATCHDOG         0x00000004
#define ERR_CLAW_ESTOP           0x00000100
#define ERR_CLAW_TEMPERATURE     0x00000200
#define ERR_CLAW_VOLTAGE_HIGH    0x00000400
#define ERR_CLAW_VOLTAGE_LOW     0x00000800
#define ERR_CLAW_DRIVER_FAULT    0x00001000
#define ERR_IMU_COMM_FAILURE     0x00010000
#define ERR_IMU_CALIBRATION      0x00020000
#define ERR_CRITICAL_MASK        0xF0000000  // Mask for critical errors
