#pragma once
#include <Arduino.h>
#include "RoboClaw.h"
#include "Constants.h"
#include "Types.h"

class RoboClawDriver {
public:
    RoboClawDriver(HardwareSerial* serial);
    
    /**
     * Initialize the RoboClaw driver
     * @return True if initialization was successful
     */
    bool initialize();
    
    /**
     * Set wheel velocities for all four wheels
     * @param fl Front left wheel velocity (encoder ticks/s)
     * @param rl Rear left wheel velocity (encoder ticks/s)
     * @param fr Front right wheel velocity (encoder ticks/s)
     * @param rr Rear right wheel velocity (encoder ticks/s)
     * @return True if commands were sent successfully
     */
    bool setWheelVelocities(int32_t fl, int32_t rl, int32_t fr, int32_t rr);
    
    /**
     * Set wheel velocities with acceleration limit
     * @param fl Front left wheel velocity (encoder ticks/s)
     * @param rl Rear left wheel velocity (encoder ticks/s)
     * @param fr Front right wheel velocity (encoder ticks/s)
     * @param rr Rear right wheel velocity (encoder ticks/s)
     * @param accel Acceleration limit (encoder ticks/s²)
     * @return True if commands were sent successfully
     */
    bool setWheelVelocitiesWithAccel(int32_t fl, int32_t rl, int32_t fr, int32_t rr, uint32_t accel);
    
    /**
     * Emergency stop - immediately stop all motors
     * @return True if commands were sent successfully
     */
    bool emergencyStop();
    
    /**
     * Reset all encoders to zero
     * @return True if commands were sent successfully
     */
    bool resetEncoders();
    
    /**
     * Get all encoder values
     * @param fl Front left encoder value (output)
     * @param rl Rear left encoder value (output)
     * @param fr Front right encoder value (output)
     * @param rr Rear right encoder value (output)
     * @return True if all values were read successfully
     */
    bool getAllEncoders(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr);
    
    /**
     * Get all wheel speeds
     * @param fl Front left wheel speed (output)
     * @param rl Rear left wheel speed (output)
     * @param fr Front right wheel speed (output)
     * @param rr Rear right wheel speed (output)
     * @return True if all values were read successfully
     */
    bool getAllSpeeds(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr);
    
    /**
     * Get all motor currents
     * @param fl Front left motor current (output)
     * @param rl Rear left motor current (output)
     * @param fr Front right motor current (output)
     * @param rr Rear right motor current (output)
     * @return True if all values were read successfully
     */
    bool getAllCurrents(int16_t& fl, int16_t& rl, int16_t& fr, int16_t& rr);
    
    /**
     * Get battery voltages
     * @param left Left side voltage (output)
     * @param right Right side voltage (output)
     * @return True if both values were read successfully
     */
    bool getVoltages(uint16_t& left, uint16_t& right);
    
    /**
     * Get error status from controllers
     * @param left Left controller errors (output)
     * @param right Right controller errors (output)
     * @return True if both values were read successfully
     */
    bool getErrors(uint32_t& left, uint32_t& right);
    
    /**
     * Get controller temperatures
     * @param left Left controller temperature (output)
     * @param right Right controller temperature (output)
     * @return True if both values were read successfully
     */
    bool getTemperatures(uint16_t& left, uint16_t& right);
    
    /**
     * Update the status array with current RoboClaw values
     * @param statusArray Array to update
     * @return True if update was successful
     */
    bool updateStatusArray(int64_t* statusArray);
    
private:
    RoboClaw roboclaw;
    uint8_t addressLeft;
    uint8_t addressRight;
    
    /**
     * Read data from the left controller
     * @return True if successful
     */
    bool readLeftController();
    
    /**
     * Read data from the right controller
     * @return True if successful
     */
    bool readRightController();
};
