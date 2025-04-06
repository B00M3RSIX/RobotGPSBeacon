#include "BeaconMicroROSInterface.h"
#include "HatchManager.h"
#include "GPSManager.h"

BeaconMicroROSInterface::BeaconMicroROSInterface(HatchManager* hatchManager, GPSManager* gpsManager)
    : hatchManager(hatchManager)
    , gpsManager(gpsManager)
    , microros_initialized(false)
    , microros_destroying(false)
    , ping_timeout_ms(100)
    , last_publish_hatch(0)
    , last_publish_gps(0)
{
}

void BeaconMicroROSInterface::initialize() {
    // Setup MicroROS transport
    set_microros_transports();
    
    // Initialisiere LED-Strip Controller
    ledAnimationController.begin();
    
    // Reset timers
    last_publish_hatch = 0;
    last_publish_gps = 0;
}

void BeaconMicroROSInterface::checkConnection() {
    if (RMW_RET_OK == rmw_uros_ping_agent(ping_timeout_ms, 1)) {
        if (!microros_initialized && !microros_destroying) {
            if (createEntities()) {
                microros_initialized = true;
                
                // Synchronize time with ROS
                rmw_uros_sync_session(20);
                
                // Reset ping timeout to default
                ping_timeout_ms = 100;
            }
        }
    } else if (microros_initialized) {
        microros_destroying = true;
        destroyEntities();
        microros_initialized = false;
        microros_destroying = false;
        
        // Set shorter ping timeout for faster reconnection
        ping_timeout_ms = 50;
    }
}

bool BeaconMicroROSInterface::processMessages() {
    if (!microros_initialized) {
        return false;
    }
    
    // Aktualisiere LED-Strip Animation (nicht-blockierend)
    ledAnimationController.update();
    
    // Verarbeite MicroROS-Nachrichten
    return (rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1)) == RCL_RET_OK);
}

bool BeaconMicroROSInterface::publishHatchStatus() {
    if (!microros_initialized || last_publish_hatch < HATCH_PUBLISH_RATE_MS) {
        return false;
    }
    
    msg_hatch_is_open.data = hatchManager->isHatchOpen();
    RCCHECK(rcl_publish(&pub_hatch_is_open, &msg_hatch_is_open, NULL));
    last_publish_hatch = 0;
    
    return true;
}

bool BeaconMicroROSInterface::publishGPSData() {
    if (!microros_initialized || last_publish_gps < GPS_PUBLISH_RATE_MS) {
        return false;
    }
    
    const NavSatFixData& navsat_data = gpsManager->getNavSatFixData();
    
    // Kopiere die Daten in die ROS-Nachricht
    msg_gps.status.status = navsat_data.status;
    msg_gps.status.service = 1;  // GPS = 1 in ROS2 NavSatFix
    
    msg_gps.latitude = navsat_data.latitude;
    msg_gps.longitude = navsat_data.longitude;
    msg_gps.altitude = navsat_data.altitude;
    
    // Kopiere die Kovarianzmatrix
    for (int i = 0; i < 9; i++) {
        msg_gps.position_covariance[i] = navsat_data.position_covariance[i];
    }
    msg_gps.position_covariance_type = navsat_data.position_covariance_type;
    
    RCCHECK(rcl_publish(&pub_gps, &msg_gps, NULL));
    last_publish_gps = 0;
    
    return true;
}

bool BeaconMicroROSInterface::isConnected() const {
    return microros_initialized;
}

bool BeaconMicroROSInterface::createEntities() {
    // Initialize allocator
    allocator = rcl_get_default_allocator();
    
    // Initialize support
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    
    // Create node
    RCCHECK(rclc_node_init_default(&node, "beacon_node", ROS_NAMESPACE, &support));
    
    // Create publishers
    RCCHECK(rclc_publisher_init_default(
        &pub_hatch_is_open,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        ROS_NAMESPACE TOPIC_HATCH_STATUS
    ));
    
    RCCHECK(rclc_publisher_init_default(
        &pub_gps,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, NavSatFix),
        ROS_NAMESPACE TOPIC_GPS
    ));
    
    // Create executor
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    
    return true;
}

bool BeaconMicroROSInterface::destroyEntities() {
    RCCHECK(rcl_publisher_fini(&pub_hatch_is_open, &node));
    RCCHECK(rcl_publisher_fini(&pub_gps, &node));
    RCCHECK(rclc_executor_fini(&executor));
    RCCHECK(rcl_node_fini(&node));
    RCCHECK(rclc_support_fini(&support));
    
    delay(100);
    return true;
}
