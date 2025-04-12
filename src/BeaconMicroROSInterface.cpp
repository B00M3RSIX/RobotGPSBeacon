#include "BeaconMicroROSInterface.h"
#include "HatchManager.h"
#include "GPSManager.h"

#define DEBUG_SERIAL Serial
#if defined DEBUG_SERIAL
  #define DEBUG_PRINT_LN(X)  DEBUG_SERIAL.println((X))
  #define DEBUG_PRINT(X)  DEBUG_SERIAL.print((X))
#else
  #define DEBUG_PRINT_LN(X)
  #define DEBUG_PRINT(X)
#endif 



BeaconMicroROSInterface::BeaconMicroROSInterface(HatchManager* hatchManager, GPSManager* gpsManager)
    : hatchManager(hatchManager)
    , gpsManager(gpsManager)
    , ping_timeout_ms(100)
    , last_publish_hatch(0)
    , last_publish_gps(0)
{
}

void BeaconMicroROSInterface::initialize() {
    // Setup MicroROS transport
    Serial2.begin(115200);
    set_microros_serial_transports(Serial2);
    
    state = WAITING_AGENT;
    ping_timeout_ms = 500;
    
    // Reset timers
    last_publish_hatch = 0;
    last_publish_gps = 0;


}

void BeaconMicroROSInterface::update() {

    switch (state) {
        case WAITING_AGENT:
          state = (RMW_RET_OK == rmw_uros_ping_agent(ping_timeout_ms, 1)) ? AGENT_AVAILABLE : WAITING_AGENT;
          break;
        case AGENT_AVAILABLE:
          state = (true == createEntities()) ? AGENT_CONNECTED : WAITING_AGENT;
          if (state == WAITING_AGENT) {
            destroyEntities();
          };
          break;
        case AGENT_CONNECTED:
          state = (RMW_RET_OK == rmw_uros_ping_agent(ping_timeout_ms, 1)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;
          if (state == AGENT_CONNECTED) {
            rmw_uros_sync_session(ping_timeout_ms);
            time_ms = rmw_uros_epoch_millis(); 
            processMessages();
          }
          break;
        case AGENT_DISCONNECTED:
          destroyEntities();
          state = WAITING_AGENT;
          break;
        default:
          break;
      }
      DEBUG_PRINT("ROS STATE: ");
      DEBUG_PRINT_LN(state);
}

bool BeaconMicroROSInterface::destroyEntities() {
    DEBUG_PRINT_LN("destroyEntities");

    rmw_context_t * rmw_context = rcl_context_get_rmw_context(&support.context);
    (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

    rcl_publisher_fini(&pub_hatch_is_open, &node);
    rcl_publisher_fini(&pub_gps, &node);
 
      //RCCHECK(rclc_executor_fini(&executor));

    rcl_node_fini(&node);
    rclc_support_fini(&support);

    DEBUG_PRINT_LN("destroyEntities END");

    return true;
}

bool BeaconMicroROSInterface::createEntities() {
    DEBUG_PRINT_LN("createEntities");
    // Initialize allocator
    allocator = rcl_get_default_allocator();
    
    // Initialize support
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    
    // Create node
    RCCHECK(rclc_node_init_default(&node, "beacon_node", "", &support));
    
    // Create hatch publishers
    RCCHECK(rclc_publisher_init_default(
        &pub_hatch_is_open,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
        ROS_NAMESPACE TOPIC_HATCH_STATUS
    ));
    // Create gps publishers
    RCCHECK(rclc_publisher_init_default(
        &pub_gps,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, NavSatFix),
        ROS_NAMESPACE TOPIC_GPS
    ));
    
    // Create executor
    //executor = rclc_executor_get_zero_initialized_executor();
    //RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    DEBUG_PRINT_LN("createEntities END");
   
    return true;
}



bool BeaconMicroROSInterface::processMessages() {
    if (state != AGENT_CONNECTED) {
        return false;
    }
    publishGPSData();
    publishHatchStatus();

    // Verarbeite MicroROS-Nachrichten
    return (rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1)) == RCL_RET_OK);
}

bool BeaconMicroROSInterface::publishHatchStatus() {
    if ((state != AGENT_CONNECTED) || last_publish_hatch < HATCH_PUBLISH_RATE_MS) {
        return false;
    }
    
    msg_hatch_is_open.data = hatchManager->isHatchOpen();
    RCCHECK(rcl_publish(&pub_hatch_is_open, &msg_hatch_is_open, NULL));
    last_publish_hatch = 0;
    
    return true;
}

bool BeaconMicroROSInterface::publishGPSData() {
    if ((state != AGENT_CONNECTED) || last_publish_gps < GPS_PUBLISH_RATE_MS) {
        return false;
    }
    
    msg_gps.header.stamp.sec  = int32_t( time_ms/1000);
    msg_gps.header.stamp.nanosec  = int32_t( time_ms % 1000)*1000;

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

BeaconMicroROSInterface::states BeaconMicroROSInterface::getConnectionState() {
    return state  ;
}


