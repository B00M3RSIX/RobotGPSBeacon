#pragma once
#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/bool.h>
#include <sensor_msgs/msg/nav_sat_fix.h>
#include "config.h"

// Forward-Deklarationen
class HatchManager;
class GPSManager;

// Error checking macros
#define RCCHECK(fn)     { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}

class BeaconMicroROSInterface {
public:

    enum states {
        WAITING_AGENT,
        AGENT_AVAILABLE,
        AGENT_CONNECTED,
        AGENT_DISCONNECTED
    } state;

    BeaconMicroROSInterface(HatchManager* hatchManager, GPSManager* gpsManager);
    
    void initialize();
    void update();
    bool processMessages();
    
    bool publishHatchStatus();
    bool publishGPSData();
    
    states getConnectionState() ;
    
private:
    HatchManager* hatchManager;
    GPSManager* gpsManager;
    
   
    // MicroROS-Entitäten
    rclc_support_t support;
    rcl_node_t node;
    rclc_executor_t executor;
    rcl_allocator_t allocator;
    
    // Publishers
    rcl_publisher_t pub_hatch_is_open;
    rcl_publisher_t pub_gps;
    
    // Messages
    std_msgs__msg__Bool msg_hatch_is_open;
    sensor_msgs__msg__NavSatFix msg_gps;
    
    // Status
    int ping_timeout_ms;
    elapsedMillis last_publish_hatch;
    elapsedMillis last_publish_gps;

    int64_t time_ms;
    
    // Hilfsmethoden
    bool createEntities();
    bool destroyEntities();
};
