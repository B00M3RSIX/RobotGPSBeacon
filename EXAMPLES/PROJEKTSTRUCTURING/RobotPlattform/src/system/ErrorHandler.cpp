#include "system/DebugUtils.h"
#include "system/ErrorHandler.h"

ErrorHandler::ErrorHandler()
    : activeErrors(ERR_NONE) {
    // Initialize error records
    for (uint8_t i = 0; i < MAX_ERROR_RECORDS; i++) {
        errorRecords[i].errorCode = 0;
        errorRecords[i].count = 0;
        errorRecords[i].lastOccurrence = 0;
        errorRecords[i].active = false;
    }
}

void ErrorHandler::registerError(uint32_t errorCode) {
    // Add error to active errors
    activeErrors |= errorCode;
    
    // Update error record
    updateErrorRecord(errorCode);
}

void ErrorHandler::clearError(uint32_t errorCode) {
    // Remove error from active errors
    activeErrors &= ~errorCode;
    
    // Update error record
    int index = findErrorRecord(errorCode);
    if (index >= 0) {
        errorRecords[index].active = false;
    }
}

void ErrorHandler::resetNonCriticalErrors() {
    // Clear all errors except critical ones
    activeErrors &= ERR_CRITICAL_MASK;
    
    // Update error records
    for (uint8_t i = 0; i < MAX_ERROR_RECORDS; i++) {
        if (errorRecords[i].errorCode != 0 && 
            !(errorRecords[i].errorCode & ERR_CRITICAL_MASK)) {
            errorRecords[i].active = false;
        }
    }
}

bool ErrorHandler::hasError(uint32_t errorCode) const {
    return (activeErrors & errorCode) != 0;
}

uint32_t ErrorHandler::getActiveErrors() const {
    return activeErrors;
}

void ErrorHandler::updateFromRoboClawErrors(uint32_t errorLeft, uint32_t errorRight) {
    // Clear existing RoboClaw errors
    activeErrors &= ~(ERR_CLAW_ESTOP | ERR_CLAW_TEMPERATURE | 
                     ERR_CLAW_VOLTAGE_HIGH | ERR_CLAW_VOLTAGE_LOW |
                     ERR_CLAW_DRIVER_FAULT);
    
    // Map RoboClaw error bits to our error codes
    
    // E-Stop (bit 0)
    if ((errorLeft & 0x01) || (errorRight & 0x01)) {
        registerError(ERR_CLAW_ESTOP);
    }
    
    // Temperature (bits 1 and 2)
    if ((errorLeft & 0x06) || (errorRight & 0x06)) {
        registerError(ERR_CLAW_TEMPERATURE);
    }
    
    // Main battery high (bit 3)
    if ((errorLeft & 0x08) || (errorRight & 0x08)) {
        registerError(ERR_CLAW_VOLTAGE_HIGH);
    }
    
    // Logic battery low (bit 5)
    if ((errorLeft & 0x20) || (errorRight & 0x20)) {
        registerError(ERR_CLAW_VOLTAGE_LOW);
    }
    
    // Motor driver fault (bits 6 and 7)
    if ((errorLeft & 0xC0) || (errorRight & 0xC0)) {
        registerError(ERR_CLAW_DRIVER_FAULT);
    }
}

ErrorRecord ErrorHandler::getErrorRecord(uint32_t errorCode) const {
    int index = findErrorRecord(errorCode);
    
    if (index >= 0) {
        return errorRecords[index];
    } else {
        // Return empty record if not found
        ErrorRecord empty = {0, 0, 0, false};
        return empty;
    }
}

void ErrorHandler::printActiveErrors(bool debugEnabled) const {
    if (!debugEnabled || activeErrors == ERR_NONE) {
        return;
    }
    
    DEBUG_PRINT("Active errors: 0x");
    DEBUG_PRINTLN(activeErrors, HEX);
    
    // Print individual error flags
    if (activeErrors & ERR_SYS_INIT_FAILURE)
        DEBUG_PRINTLN("- System initialization failure");
    if (activeErrors & ERR_SYS_COMM_TIMEOUT)
        DEBUG_PRINTLN("- Communication timeout");
    if (activeErrors & ERR_SYS_WATCHDOG)
        DEBUG_PRINTLN("- Watchdog timeout");
    if (activeErrors & ERR_CLAW_ESTOP)
        DEBUG_PRINTLN("- Emergency stop active");
    if (activeErrors & ERR_CLAW_TEMPERATURE)
        DEBUG_PRINTLN("- Controller temperature high");
    if (activeErrors & ERR_CLAW_VOLTAGE_HIGH)
        DEBUG_PRINTLN("- Battery voltage high");
    if (activeErrors & ERR_CLAW_VOLTAGE_LOW)
        DEBUG_PRINTLN("- Battery voltage low");
    if (activeErrors & ERR_CLAW_DRIVER_FAULT)
        DEBUG_PRINTLN("- Motor driver fault");
    if (activeErrors & ERR_IMU_COMM_FAILURE)
        DEBUG_PRINTLN("- IMU communication failure");
    if (activeErrors & ERR_IMU_CALIBRATION)
        DEBUG_PRINTLN("- IMU calibration error");
}

int ErrorHandler::findErrorRecord(uint32_t errorCode) const {
    // Look for existing record
    for (uint8_t i = 0; i < MAX_ERROR_RECORDS; i++) {
        if (errorRecords[i].errorCode == errorCode) {
            return i;
        }
    }
    
    // Look for empty slot
    for (uint8_t i = 0; i < MAX_ERROR_RECORDS; i++) {
        if (errorRecords[i].errorCode == 0) {
            return i;
        }
    }
    
    // If no slot found, return -1
    return -1;
}

void ErrorHandler::updateErrorRecord(uint32_t errorCode) {
    int index = findErrorRecord(errorCode);
    
    if (index >= 0) {
        // Update existing record
        errorRecords[index].errorCode = errorCode;
        errorRecords[index].count++;
        errorRecords[index].lastOccurrence = millis();
        errorRecords[index].active = true;
    } else {
        // Find least recently used record to replace
        uint32_t oldestTime = UINT32_MAX;
        int oldestIndex = 0;
        
        for (uint8_t i = 0; i < MAX_ERROR_RECORDS; i++) {
            if (errorRecords[i].lastOccurrence < oldestTime) {
                oldestTime = errorRecords[i].lastOccurrence;
                oldestIndex = i;
            }
        }
        
        // Replace oldest record
        errorRecords[oldestIndex].errorCode = errorCode;
        errorRecords[oldestIndex].count = 1;
        errorRecords[oldestIndex].lastOccurrence = millis();
        errorRecords[oldestIndex].active = true;
    }
}