#include "system/DebugUtils.h"
#include "hardware/IMUDriver.h"
#include <sensor_msgs/msg/imu.h>
#include <micro_ros_utilities/string_utilities.h>

IMUDriver::IMUDriver() 
    : bno(55, 0x28),  // Default BNO055 address is 0x28
      isInitialized(false) {
}

bool IMUDriver::initialize() {
    // Initialize IMU
    if (!bno.begin()) {
        DEBUG_PRINTLN("Failed to initialize BNO055");
        return false;
    }
    
    // Set up IMU for external crystal for better accuracy
    bno.setExtCrystalUse(true);
    
    // Display sensor details
    displaySensorDetails();
    
    // Load calibration if available
    loadCalibration();
    
    isInitialized = true;
    return true;
}

bool IMUDriver::getQuaternion(float& w, float& x, float& y, float& z) {
    if (!isInitialized) return false;
    
    imu::Quaternion quat = bno.getQuat();
    w = quat.w();
    x = quat.x();
    y = quat.y();
    z = quat.z();
    
    return true;
}

bool IMUDriver::getEulerAngles(float& roll, float& pitch, float& yaw) {
    if (!isInitialized) return false;
    
    sensors_event_t event;
    bno.getEvent(&event);
    
    roll = event.orientation.roll;
    pitch = event.orientation.pitch;
    yaw = event.orientation.heading;
    
    return true;
}

bool IMUDriver::getLinearAcceleration(float& x, float& y, float& z) {
    if (!isInitialized) return false;
    
    imu::Vector<3> linear_accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    x = linear_accel.x();
    y = linear_accel.y();
    z = linear_accel.z();
    
    return true;
}

bool IMUDriver::getAngularVelocity(float& x, float& y, float& z) {
    if (!isInitialized) return false;
    
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    x = gyro.x();
    y = gyro.y();
    z = gyro.z();
    
    return true;
}

IMUCalibrationStatus IMUDriver::getCalibrationStatus() {
    IMUCalibrationStatus status = {0, 0, 0, 0}; // Default to uncalibrated
    
    if (!isInitialized) return status;
    
    // Get calibration values (0-3, 3 = fully calibrated)
    bno.getCalibration(&status.system, &status.gyro, &status.accel, &status.mag);
    
    return status;
}

bool IMUDriver::isCalibrated() {
    if (!isInitialized) return false;
    
    IMUCalibrationStatus status = getCalibrationStatus();
    
    // System must be calibrated (3) and all sensors at least mostly calibrated (2+)
    return (status.system == 3 && status.gyro >= 2 && status.accel >= 2 && status.mag >= 2);
}

bool IMUDriver::hasSavedCalibration() {
    int eeAddress = 0;
    long bnoID;
    EEPROM.get(eeAddress, bnoID);
    
    sensor_t sensor;
    bno.getSensor(&sensor);
    
    return (bnoID == sensor.sensor_id);
}

bool IMUDriver::loadCalibration() {
    if (!isInitialized) return false;
    
    int eeAddress = 0;
    long bnoID;
    EEPROM.get(eeAddress, bnoID);
    
    sensor_t sensor;
    bno.getSensor(&sensor);
    
    if (bnoID != sensor.sensor_id) {
        DEBUG_PRINTLN("No calibration data found in EEPROM");
        return false;
    }
    
    DEBUG_PRINTLN("Found calibration data, loading...");
    
    eeAddress += sizeof(long);
    adafruit_bno055_offsets_t calibrationData;
    EEPROM.get(eeAddress, calibrationData);
    
    // Display the calibration data
    displaySensorOffsets(calibrationData);
    
    // Set the calibration data
    bno.setSensorOffsets(calibrationData);
    
    DEBUG_PRINTLN("Calibration data loaded");
    return true;
}

bool IMUDriver::saveCalibration() {
    if (!isInitialized) return false;
    
    DEBUG_PRINTLN("Saving calibration data to EEPROM...");
    
    // Get the sensor ID
    int eeAddress = 0;
    sensor_t sensor;
    bno.getSensor(&sensor);
    
    // Save the sensor ID to EEPROM
    EEPROM.put(eeAddress, sensor.sensor_id);
    eeAddress += sizeof(long);
    
    // Get the current calibration data
    adafruit_bno055_offsets_t calibrationData;
    bno.getSensorOffsets(calibrationData);
    
    // Save the calibration data to EEPROM
    EEPROM.put(eeAddress, calibrationData);
    
    DEBUG_PRINTLN("Calibration data saved");
    return true;
}

bool IMUDriver::updateIMUMessage(void* msgImu_void, int64_t timestamp) {
    if (!isInitialized || !msgImu_void) return false;
    
    // Cast to the correct type
    auto msgImu = static_cast<sensor_msgs__msg__Imu*>(msgImu_void);
    
    // Set the timestamp
    msgImu->header.stamp.sec = timestamp / 1000;
    msgImu->header.stamp.nanosec = (timestamp % 1000) * 1000000;
    msgImu->header.frame_id = micro_ros_string_utilities_set(msgImu->header.frame_id, "imu_link");
    
    // Get orientation as quaternion
    float w, x, y, z;
    if (!getQuaternion(w, x, y, z)) {
        return false;
    }
    
    msgImu->orientation.w = w;
    msgImu->orientation.x = x;
    msgImu->orientation.y = y;
    msgImu->orientation.z = z;
    
    // Get angular velocity (rad/s)
    float gyro_x, gyro_y, gyro_z;
    if (getAngularVelocity(gyro_x, gyro_y, gyro_z)) {
        // Convert from deg/s to rad/s
        msgImu->angular_velocity.x = gyro_x * M_PI / 180.0;
        msgImu->angular_velocity.y = gyro_y * M_PI / 180.0;
        msgImu->angular_velocity.z = gyro_z * M_PI / 180.0;
    } else {
        // Set to zero if reading fails
        msgImu->angular_velocity.x = 0;
        msgImu->angular_velocity.y = 0;
        msgImu->angular_velocity.z = 0;
    }
    
    // Get linear acceleration (m/s^2)
    float accel_x, accel_y, accel_z;
    if (getLinearAcceleration(accel_x, accel_y, accel_z)) {
        msgImu->linear_acceleration.x = accel_x;
        msgImu->linear_acceleration.y = accel_y;
        msgImu->linear_acceleration.z = accel_z;
    } else {
        // Set to zero if reading fails
        msgImu->linear_acceleration.x = 0;
        msgImu->linear_acceleration.y = 0;
        msgImu->linear_acceleration.z = 0;
    }
    
    return true;
}

void IMUDriver::displaySensorDetails() {
    sensor_t sensor;
    bno.getSensor(&sensor);
    
    DEBUG_PRINTLN("------------------------------------");
    DEBUG_PRINT("Sensor:       "); DEBUG_PRINTLN(sensor.name);
    DEBUG_PRINT("Driver Ver:   "); DEBUG_PRINTLN(sensor.version);
    DEBUG_PRINT("Unique ID:    "); DEBUG_PRINTLN(sensor.sensor_id);
    DEBUG_PRINT("Max Value:    "); DEBUG_PRINT(sensor.max_value); DEBUG_PRINTLN(" xxx");
    DEBUG_PRINT("Min Value:    "); DEBUG_PRINT(sensor.min_value); DEBUG_PRINTLN(" xxx");
    DEBUG_PRINT("Resolution:   "); DEBUG_PRINT(sensor.resolution); DEBUG_PRINTLN(" xxx");
    DEBUG_PRINTLN("------------------------------------");
}

void IMUDriver::displaySensorOffsets(const adafruit_bno055_offsets_t& calibData) {
    DEBUG_PRINTLN("Calibration values:");
    
    DEBUG_PRINT("Accelerometer: ");
    DEBUG_PRINT(calibData.accel_offset_x); DEBUG_PRINT(" ");
    DEBUG_PRINT(calibData.accel_offset_y); DEBUG_PRINT(" ");
    DEBUG_PRINTLN(calibData.accel_offset_z);
    
    DEBUG_PRINT("Gyroscope: ");
    DEBUG_PRINT(calibData.gyro_offset_x); DEBUG_PRINT(" ");
    DEBUG_PRINT(calibData.gyro_offset_y); DEBUG_PRINT(" ");
    DEBUG_PRINTLN(calibData.gyro_offset_z);
    
    DEBUG_PRINT("Magnetometer: ");
    DEBUG_PRINT(calibData.mag_offset_x); DEBUG_PRINT(" ");
    DEBUG_PRINT(calibData.mag_offset_y); DEBUG_PRINT(" ");
    DEBUG_PRINTLN(calibData.mag_offset_z);
    
    DEBUG_PRINT("Accel Radius: ");
    DEBUG_PRINTLN(calibData.accel_radius);
    
    DEBUG_PRINT("Mag Radius: ");
    DEBUG_PRINTLN(calibData.mag_radius);
}

void IMUDriver::printCalibrationDetails() {
    IMUCalibrationStatus status = getCalibrationStatus();
    
    DEBUG_PRINT("Calibration: Sys=");
    DEBUG_PRINT(status.system);
    DEBUG_PRINT(" Gyro=");
    DEBUG_PRINT(status.gyro);
    DEBUG_PRINT(" Accel=");
    DEBUG_PRINT(status.accel);
    DEBUG_PRINT(" Mag=");
    DEBUG_PRINTLN(status.mag);
    
    DEBUG_PRINTLN("0 = uncalibrated, 3 = fully calibrated");
}
