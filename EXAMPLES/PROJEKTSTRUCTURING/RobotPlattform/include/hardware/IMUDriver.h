#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "Types.h"

class IMUDriver {
public:
    IMUDriver();
    
    /**
     * Initialize the IMU
     * @return True if initialization was successful
     */
    bool initialize();
    
    /**
     * Get quaternion orientation from IMU
     * @param w W component (output)
     * @param x X component (output)
     * @param y Y component (output)
     * @param z Z component (output)
     * @return True if read was successful
     */
    bool getQuaternion(float& w, float& x, float& y, float& z);
    
    /**
     * Get euler angles (roll, pitch, yaw) from IMU
     * @param roll Roll angle (output)
     * @param pitch Pitch angle (output)
     * @param yaw Yaw angle (output)
     * @return True if read was successful
     */
    bool getEulerAngles(float& roll, float& pitch, float& yaw);
    
    /**
     * Get linear acceleration from IMU
     * @param x X acceleration (output)
     * @param y Y acceleration (output)
     * @param z Z acceleration (output)
     * @return True if read was successful
     */
    bool getLinearAcceleration(float& x, float& y, float& z);
    
    /**
     * Get angular velocity from IMU
     * @param x X angular velocity (output)
     * @param y Y angular velocity (output)
     * @param z Z angular velocity (output)
     * @return True if read was successful
     */
    bool getAngularVelocity(float& x, float& y, float& z);
    
    /**
     * Get IMU calibration status
     * @return Calibration status structure
     */
    IMUCalibrationStatus getCalibrationStatus();
    
    /**
     * Check if IMU is fully calibrated
     * @return True if fully calibrated
     */
    bool isCalibrated();
    
    /**
     * Load calibration data from EEPROM
     * @return True if calibration was loaded successfully
     */
    bool loadCalibration();
    
    /**
     * Save current calibration to EEPROM
     * @return True if calibration was saved successfully
     */
    bool saveCalibration();
    
    /**
     * Update IMU message with current orientation
     * @param msgImu Message to update (sensor_msgs__msg__Imu*)
     * @param timestamp Current timestamp
     * @return True if update was successful
     */
    bool updateIMUMessage(void* msgImu, int64_t timestamp);
    
    /**
     * Print detailed calibration information to Serial
     */
    void printCalibrationDetails();
    
private:
    Adafruit_BNO055 bno;
    bool isInitialized;
    
    /**
     * Check if a saved calibration exists in EEPROM
     * @return True if calibration exists
     */
    bool hasSavedCalibration();
    
    /**
     * Display sensor information
     */
    void displaySensorDetails();
    
    /**
     * Display current sensor offsets
     * @param calibData Calibration data to display
     */
    void displaySensorOffsets(const adafruit_bno055_offsets_t& calibData);
};
