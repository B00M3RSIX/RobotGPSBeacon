#pragma once

// Publish-Raten (in Millisekunden)
#define HATCH_PUBLISH_RATE_MS 500
#define GPS_PUBLISH_RATE_MS 500
#define LED_ANIMATION_FEEDBACK_RATE_MS 200

// Pin-Definitionen
#define HATCH_LEFT_PIN 2
#define HATCH_RIGHT_PIN 3
#define GPS_SERIAL Serial3
#define GPS_BAUD 38400

// LED-Statusanzeige
#define LED_STATUS_CONNECTING_R 0
#define LED_STATUS_CONNECTING_G 255
#define LED_STATUS_CONNECTING_B 255  // Cyan
#define LED_STATUS_CONNECTED_NO_FIX_R 255
#define LED_STATUS_CONNECTED_NO_FIX_G 255
#define LED_STATUS_CONNECTED_NO_FIX_B 0  // Gelb
#define LED_STATUS_CONNECTED_FIX_R 0
#define LED_STATUS_CONNECTED_FIX_G 0
#define LED_STATUS_CONNECTED_FIX_B 255  // Blau
#define LED_STATUS_ERROR_R 255
#define LED_STATUS_ERROR_G 0
#define LED_STATUS_ERROR_B 0  // Rot

// Blink-Intervalle (in Millisekunden)
#define BLINK_INTERVAL_CONNECTING 500
#define BLINK_INTERVAL_NO_FIX 1000
#define BLINK_INTERVAL_ERROR 250

// ROS-Topics
#define ROS_NAMESPACE "/Beacon/"
#define TOPIC_HATCH_STATUS "hatchIsOpen"
#define TOPIC_GPS "gps"
#define ACTION_LED_ANIMATION "led_animation"
