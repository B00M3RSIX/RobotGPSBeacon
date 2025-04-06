#pragma once

#include <Arduino.h>
#include "hardware/IOManager.h"

// External declaration of the global IOManager pointer
extern IOManager* ioManager;

// Debug configuration
const unsigned long DEBUG_CHECK_INTERVAL = 1000; // ms between debug state checks

// Serial initialization and debug state cache - keep in header
namespace {
    volatile bool serialInitialized = false;
    volatile bool cachedDebugState = false;
    unsigned long lastDebugCheck = 0;
}

// Force an immediate check of debug state
inline void forceDebugCheck() {
    cachedDebugState = ioManager->isDebugModeEnabled();
    lastDebugCheck = millis();
}

// Initialize debug utilities
inline void initDebugUtils() {
    forceDebugCheck();
}

// Check if debug is enabled, with state caching for performance
inline bool isDebugEnabled() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastDebugCheck >= DEBUG_CHECK_INTERVAL) {
        forceDebugCheck();
    }
    return cachedDebugState;
}

// Ensure Serial is initialized only once
inline void ensureSerialInitialized() {
    if (!serialInitialized) {
        Serial.begin(115200);
        serialInitialized = true;
    }
}

// Standard debug macros
#define DEBUG_PRINT(...) do { \
    if (isDebugEnabled()) { \
        ensureSerialInitialized(); \
        Serial.print(__VA_ARGS__); \
    } \
} while(0)

#define DEBUG_PRINTLN(...) do { \
    if (isDebugEnabled()) { \
        ensureSerialInitialized(); \
        Serial.println(__VA_ARGS__); \
    } \
} while(0)

// Batch debug macros for optimized blocks
#define DEBUG_BEGIN_BLOCK() \
    if (isDebugEnabled()) { \
        ensureSerialInitialized(); \
        {

#define DEBUG_END_BLOCK() \
        } \
    }

#define DEBUG_PRINT_DIRECT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN_DIRECT(...) Serial.println(__VA_ARGS__)