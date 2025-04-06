#pragma once
#include <Arduino.h>
#include <micro_ros_arduino.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int64_multi_array.h>
#include <std_msgs/msg/u_int32.h>
#include <std_msgs/msg/u_int8.h>
#include <sensor_msgs/msg/imu.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <micro_ros_utilities/string_utilities.h>
#include "Types.h"

// Forward declarations
class CommandProcessor;
class IMUDriver;
class RoboClawDriver;

// Error checking macros
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}
//#define RCCHECK_VOID(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}

class MicroROSInterface {
public:
    MicroROSInterface(CommandProcessor* commandProcessor, 
                     IMUDriver* imuDriver,
                     RoboClawDriver* roboClawDriver);
    
    /**
     * Initialize the MicroROS interface
     */
    void initialize();
    
    /**
     * Check connection to ROS agent
     */
    void checkConnection();
    
    /**
     * Process incoming messages
     * @return True if messages were processed
     */
    bool processMessages();
    
    /**
     * Publish status array to ROS
     * @param statusArray Status array to publish
     * @return True if publish was successful
     */
    bool publishStatus(const int64_t* statusArray);
    
    /**
     * Publish IMU data to ROS
     * @return True if publish was successful
     */
    bool publishIMU();
    
    /**
     * Publish robot state to ROS
     * @param currentState Current system state
     * @param activeErrors Active error bitfield
     * @param debugMode Whether debug mode is enabled
     * @param errorResetActive Whether error reset is active
     * @return True if publish was successful
     */
    bool publishState(SystemState currentState, uint32_t activeErrors, 
                     bool debugMode, bool errorResetActive);
    
    /**
     * Check if connected to ROS
     * @return True if connected
     */
    bool isConnected() const;
    
private:
    CommandProcessor* commandProcessor;
    IMUDriver* imuDriver;
    RoboClawDriver* roboClawDriver;
    
    rclc_support_t support;
    rcl_node_t node;
    rclc_executor_t executor;
    rcl_allocator_t allocator;
    
    // Publishers
    rcl_publisher_t pub_status;
    rcl_publisher_t pub_imu;
    rcl_publisher_t pub_state;
    rcl_publisher_t pub_errors;
    
    // Subscribers
    rcl_subscription_t sub_wheel_cmd;
    rcl_subscription_t sub_robot_cmd;
    
    // Messages
    std_msgs__msg__Int64MultiArray msg_status;
    sensor_msgs__msg__Imu msg_imu;
    std_msgs__msg__UInt8 msg_state;
    std_msgs__msg__UInt32 msg_errors;
    std_msgs__msg__Int32MultiArray msg_wheel_cmd;
    std_msgs__msg__UInt32 msg_robot_cmd;
    
    // State
    bool microros_initialized;
    bool microros_destroying;
    int ping_timeout_ms;
    elapsedMillis last_publish_status;
    elapsedMillis last_publish_imu;
    elapsedMillis last_publish_state;
    
    /**
     * Create MicroROS entities (node, publishers, subscribers)
     * @return True if creation was successful
     */
    bool createEntities();
    
    /**
     * Destroy MicroROS entities
     */
    bool destroyEntities();
    
    // Static callback functions (need to be static for rclc_executor)
    static void wheel_cmd_callback(const void* msgin, void* userdata);
    static void robot_cmd_callback(const void* msgin, void* userdata);
};