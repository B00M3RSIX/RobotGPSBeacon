#ifndef BEACON_INTERFACES__ACTION__BEACON_ANIMATION_H_
#define BEACON_INTERFACES__ACTION__BEACON_ANIMATION_H_

#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rosidl_runtime_c/message_type_support_struct.h>

// Goal Definition
typedef struct beacon_interfaces__action__BeaconAnimation_Goal {
    uint8_t red;          // PARAM_RED
    uint8_t green;        // PARAM_GREEN
    uint8_t blue;         // PARAM_BLUE
    uint8_t brightness;   // PARAM_BRIGHTNESS
    uint8_t cmd;          // PARAM_CMD (AnimationCommand enum)
    uint8_t speed;        // PARAM_SPEED
    uint8_t modifier;     // PARAM_MODIFIER (Bitmask using ParameterBits)
    uint8_t multiuse1;    // PARAM_MULTIUSE1 (Meaning depends on cmd)
} beacon_interfaces__action__BeaconAnimation_Goal;

// Result Definition
typedef struct beacon_interfaces__action__BeaconAnimation_Result {
    bool success;         // True if the goal was accepted and completed/cancelled successfully
    rosidl_runtime_c__String message;  // Optional status message
} beacon_interfaces__action__BeaconAnimation_Result;

// Feedback Definition
typedef struct beacon_interfaces__action__BeaconAnimation_Feedback {
    uint8_t status;       // Current AnimationStatus enum value
    float progress;       // Animation progress (0.0 to 1.0)
} beacon_interfaces__action__BeaconAnimation_Feedback;

// Type Support Declarations
ROSIDL_TYPESUPPORT_INTERFACE_PACKAGE_DECLARATION(beacon_interfaces);

const rosidl_action_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__ACTION_SYMBOL_NAME(
    rosidl_typesupport_c,
    beacon_interfaces,
    action,
    BeaconAnimation
);

#endif  // BEACON_INTERFACES__ACTION__BEACON_ANIMATION_H_
