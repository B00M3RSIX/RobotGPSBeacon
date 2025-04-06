#include "system/DebugUtils.h"
#include "communication/MicroROSInterface.h"
#include "application/CommandProcessor.h"
#include "hardware/IMUDriver.h"
#include "hardware/RoboClawDriver.h"
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <micro_ros_utilities/type_utilities.h>
#include <micro_ros_utilities/string_utilities.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <std_msgs/msg/int64_multi_array.h>

MicroROSInterface::MicroROSInterface(CommandProcessor* commandProcessor, 
                                   IMUDriver* imuDriver,
                                   RoboClawDriver* roboClawDriver)
    : commandProcessor(commandProcessor),
      imuDriver(imuDriver),
      roboClawDriver(roboClawDriver),
      microros_initialized(false),
      microros_destroying(false),
      ping_timeout_ms(100) {
}

void MicroROSInterface::initialize() {
    // Setup MicroROS transport
    set_microros_transports();
    
    // Reset timers
    last_publish_status = 0;
    last_publish_imu = 0;
    last_publish_state = 0;
}

void MicroROSInterface::checkConnection() {
    if (RMW_RET_OK == rmw_uros_ping_agent(ping_timeout_ms, 1)) {
        if (!microros_initialized && !microros_destroying) {
            DEBUG_PRINTLN("Creating MicroROS entities...");
            if (createEntities()) {
                DEBUG_PRINTLN("MicroROS entities created successfully");
                microros_initialized = true;
                
                // Synchronize time with ROS
                rmw_uros_sync_session(20);
                
                // Reset ping timeout to default
                ping_timeout_ms = 100;
            }
        }
    } else if (microros_initialized) {
        microros_destroying = true;
        DEBUG_PRINTLN("Destroying MicroROS entities...");
        destroyEntities();
        microros_initialized = false;
        microros_destroying = false;
        
        // Set shorter ping timeout for faster reconnection
        ping_timeout_ms = 50;
    }
}
bool MicroROSInterface::createEntities() {
    // Initialize allocator
    allocator = rcl_get_default_allocator();
    
    // Initialize support
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    
    // Create node
    RCCHECK(rclc_node_init_default(&node, "MicroROS", "", &support));
    
    // Create publishers
    RCCHECK(rclc_publisher_init_best_effort(
        &pub_status,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray),
        "MCUCore/roboclawstatus"
    ));
    
    RCCHECK(rclc_publisher_init_best_effort(
        &pub_imu,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
        "MCUCore/Imu"
    ));
    
    RCCHECK(rclc_publisher_init_default(
        &pub_state,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt8),
        "MCUCore/state"
    ));
    
    RCCHECK(rclc_publisher_init_default(
        &pub_errors,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt32),
        "MCUCore/errors"
    ));
    
    // Create subscribers
    RCCHECK(rclc_subscription_init_default(
        &sub_wheel_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
        "MCUCore/wheel_cmd"
    ));
    
    RCCHECK(rclc_subscription_init_default(
        &sub_robot_cmd,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt32),
        "MCUCore/robo_cmd"
    ));
    
    // Create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
    
    // Add subscribers to executor with userdata parameter
    RCCHECK(rclc_executor_add_subscription_with_context(
        &executor,
        &sub_wheel_cmd,
        &msg_wheel_cmd,
        &MicroROSInterface::wheel_cmd_callback,
        this,
        ON_NEW_DATA
    ));
    
    RCCHECK(rclc_executor_add_subscription_with_context(
        &executor,
        &sub_robot_cmd,
        &msg_robot_cmd,
        &MicroROSInterface::robot_cmd_callback,
        this,
        ON_NEW_DATA
    ));
      
    // Initialize status message memory
    static micro_ros_utilities_memory_conf_t conf = {0};
    micro_ros_utilities_memory_rule_t rules[] = {
        {"data", 1},
        {"data.data", STATUS_ARRAY_SIZE}
    };
    conf.rules = rules;
    conf.n_rules = sizeof(rules) / sizeof(rules[0]);
    
    micro_ros_utilities_create_message_memory(
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray),
        &msg_status,
        conf
    );
    
    
    // Initialize message manually for now
    msg_status.data.capacity = STATUS_ARRAY_SIZE;
    msg_status.data.size = 0;
    msg_status.data.data = (int64_t*)malloc(STATUS_ARRAY_SIZE * sizeof(int64_t));
    
    return true;
}

bool MicroROSInterface::destroyEntities() {
    RCCHECK(rcl_publisher_fini(&pub_status, &node));
    RCCHECK(rcl_publisher_fini(&pub_imu, &node));
    RCCHECK(rcl_publisher_fini(&pub_state, &node));
    RCCHECK(rcl_publisher_fini(&pub_errors, &node));
    
    RCCHECK(rcl_subscription_fini(&sub_wheel_cmd, &node));
    RCCHECK(rcl_subscription_fini(&sub_robot_cmd, &node));
    RCCHECK(rclc_executor_fini(&executor));
    RCCHECK(rclc_support_fini(&support));
    
    delay(100);
    return false;
}

bool MicroROSInterface::processMessages() {
    if (!microros_initialized) {
        return false;
    }
    
    return (rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1)) == RCL_RET_OK);
}

bool MicroROSInterface::publishStatus(const int64_t* statusArray) {
    if (!microros_initialized || last_publish_status < STATUS_PUBLISH_INTERVAL_MS) {
        return false;
    }
    
    // Copy status array to message
    msg_status.data.size = STATUS_ARRAY_SIZE;
    memcpy(msg_status.data.data, statusArray, STATUS_ARRAY_SIZE * sizeof(int64_t));
    
    // Publish message
    RCCHECK(rcl_publish(&pub_status, &msg_status, NULL));
    last_publish_status = 0;
    
    return true;
}

bool MicroROSInterface::publishIMU() {
    if (!microros_initialized || last_publish_imu < IMU_PUBLISH_INTERVAL_MS) {
        return false;
    }
    
    // Update IMU message
    int64_t timestamp = rmw_uros_epoch_millis();
    if (imuDriver->updateIMUMessage(&msg_imu, timestamp)) {
        // Publish message
        RCCHECK(rcl_publish(&pub_imu, &msg_imu, NULL));
        last_publish_imu = 0;
        return true;
    }
    
    return false;
}

bool MicroROSInterface::publishState(SystemState currentState, uint32_t activeErrors, 
                                   bool debugMode, bool errorResetActive) {
    if (!microros_initialized || last_publish_state < 100) { // Publish every 100ms
        return false;
    }
    
    // Update state message
    msg_state.data = static_cast<uint8_t>(currentState);
    RCCHECK(rcl_publish(&pub_state, &msg_state, NULL));
    
    // Update error message
    msg_errors.data = activeErrors;
    RCCHECK(rcl_publish(&pub_errors, &msg_errors, NULL));
    
    last_publish_state = 0;
    return true;
}

bool MicroROSInterface::isConnected() const {
    return microros_initialized;
}

void MicroROSInterface::wheel_cmd_callback(const void* msgin, void* userdata) {
    MicroROSInterface* interface = static_cast<MicroROSInterface*>(userdata);
    const std_msgs__msg__Int32MultiArray* msg = static_cast<const std_msgs__msg__Int32MultiArray*>(msgin);   
    int32_t CmdVel[CMD_VEL_ARRAY_SIZE] = {0};
    memcpy( CmdVel,msg->data.data, CMD_VEL_ARRAY_SIZE * sizeof(int32_t));
    if (interface && interface->commandProcessor) {
        interface->commandProcessor->handleWheelVelocityCommand(CmdVel);
    }
}

void MicroROSInterface::robot_cmd_callback(const void* msgin, void* userdata) {
    MicroROSInterface* interface = static_cast<MicroROSInterface*>(userdata);
    const std_msgs__msg__UInt32* msg = static_cast<const std_msgs__msg__UInt32*>(msgin);
    
    if (interface && interface->commandProcessor) {
        interface->commandProcessor->handleRobotCommand(msg->data);
    }
}
