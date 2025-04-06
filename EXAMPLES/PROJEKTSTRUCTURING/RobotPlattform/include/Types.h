#pragma once
#include <stdint.h>

// System States
enum class SystemState {
    STATE_INITIALIZING,
    STATE_CALIBRATING,
    STATE_STANDBY,
    STATE_OPERATIONAL,
    STATE_ERROR,
    STATE_EMERGENCY_STOP
};

// LED Patterns
enum class LEDPattern {
    LED_OFF,
    LED_ON,
    LED_SLOW_BLINK,
    LED_FAST_BLINK,
    LED_DOUBLE_BLINK,
    LED_HEARTBEAT,
    LED_PULSE
};
/*
// Wheel Velocity Command Structure
typedef struct {
    int32_t velocities[4];  // FL, RL, FR, RR
    uint16_t acceleration;  // Optional acceleration limit
    uint8_t command_flags;  // Control flags
} WheelVelocityMsg;*/

// Error Handler Structure
typedef struct {
    uint32_t errorCode;       // Error code
    uint32_t count;           // Occurrence count
    uint32_t lastOccurrence;  // Timestamp of last occurrence
    bool active;              // Error is currently active
} ErrorRecord;

// IMU Calibration Status
typedef struct {
    uint8_t system;    // 0-3, 3 = fully calibrated
    uint8_t gyro;      // 0-3, 3 = fully calibrated
    uint8_t accel;     // 0-3, 3 = fully calibrated
    uint8_t mag;       // 0-3, 3 = fully calibrated
} IMUCalibrationStatus;