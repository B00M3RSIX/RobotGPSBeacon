#pragma once
#include <Arduino.h>
#include "Constants.h"
#include "Types.h"

class ErrorHandler {
public:
    ErrorHandler();
    
    /**
     * Register a new error
     * @param errorCode Error code to register
     */
    void registerError(uint32_t errorCode);
    
    /**
     * Clear a specific error
     * @param errorCode Error code to clear
     */
    void clearError(uint32_t errorCode);
    
    /**
     * Reset all non-critical errors
     */
    void resetNonCriticalErrors();
    
    /**
     * Check if a specific error is active
     * @param errorCode Error code to check
     * @return True if the error is active
     */
    bool hasError(uint32_t errorCode) const;
    
    /**
     * Get bitfield of all active errors
     * @return Active error bitfield
     */
    uint32_t getActiveErrors() const;
    
    /**
     * Map RoboClaw error codes to system error codes
     * @param errorLeft Left controller error code
     * @param errorRight Right controller error code
     */
    void updateFromRoboClawErrors(uint32_t errorLeft, uint32_t errorRight);
    
    /**
     * Get error record for a specific error
     * @param errorCode Error code to get record for
     * @return Error record
     */
    ErrorRecord getErrorRecord(uint32_t errorCode) const;
    
    /**
     * Print active errors to Serial if debug is enabled
     * @param debugEnabled Whether debug output is enabled
     */
    void printActiveErrors(bool debugEnabled) const;
    
private:
    // Active error bitfield
    uint32_t activeErrors;
    
    // Error records for tracking occurrences
    static const uint8_t MAX_ERROR_RECORDS = 16;
    ErrorRecord errorRecords[MAX_ERROR_RECORDS];
    
    /**
     * Find error record index by code
     * @param errorCode Error code to find
     * @return Index of error record, or -1 if not found
     */
    int findErrorRecord(uint32_t errorCode) const;
    
    /**
     * Add or update error record
     * @param errorCode Error code to record
     */
    void updateErrorRecord(uint32_t errorCode);
};