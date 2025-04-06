#include "hardware/RoboClawDriver.h"

RoboClawDriver::RoboClawDriver(HardwareSerial* serial) 
    : roboclaw(serial, ROBOCLAW_TIMEOUT),
      addressLeft(ROBOCLAW_ADDRESS_LEFT),
      addressRight(ROBOCLAW_ADDRESS_RIGHT) {
}

bool RoboClawDriver::initialize() {
    roboclaw.begin(ROBOCLAW_BAUD_RATE);
    
    // Set voltage limits
    bool success = true;
    
    if (!roboclaw.SetMainVoltages(addressLeft, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE)) {
        success = false;
    }
    
    if (!roboclaw.SetMainVoltages(addressRight, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE)) {
        success = false;
    }
    
    // Set default acceleration
    if (!roboclaw.SetM1DefaultAccel(addressLeft, DEFAULT_ACCELERATION)) {
        success = false;
    }
    
    if (!roboclaw.SetM2DefaultAccel(addressLeft, DEFAULT_ACCELERATION)) {
        success = false;
    }
    
    if (!roboclaw.SetM1DefaultAccel(addressRight, DEFAULT_ACCELERATION)) {
        success = false;
    }
    
    if (!roboclaw.SetM2DefaultAccel(addressRight, DEFAULT_ACCELERATION)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::setWheelVelocities(int32_t fl, int32_t rl, int32_t fr, int32_t rr) {
    bool success = true;
    
    // Left side motors
    if (!roboclaw.SpeedM1M2(addressLeft, rl, fl)) {
        success = false;
    }
    
    // Right side motors
    if (!roboclaw.SpeedM1M2(addressRight, fr, rr)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::setWheelVelocitiesWithAccel(int32_t fl, int32_t rl, int32_t fr, int32_t rr, uint32_t accel) {
    bool success = true;
    
    // Left side motors with acceleration
    if (!roboclaw.SpeedAccelM1M2(addressLeft, accel, rl, fl)) {
        success = false;
    }
    
    // Right side motors with acceleration
    if (!roboclaw.SpeedAccelM1M2(addressRight, accel, fr, rr)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::emergencyStop() {
    bool success = true;
    
    // Set duty cycle to 0 for immediate stop
    if (!roboclaw.DutyM1M2(addressLeft, 0, 0)) {
        success = false;
    }
    
    if (!roboclaw.DutyM1M2(addressRight, 0, 0)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::resetEncoders() {
    bool success = true;
    
    if (!roboclaw.ResetEncoders(addressLeft)) {
        success = false;
    }
    
    if (!roboclaw.ResetEncoders(addressRight)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::getAllEncoders(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr) {
    bool success = true;
    
    // Read encoders from left controller
    if (!roboclaw.ReadEncoders(addressLeft, rl, fl)) {
        success = false;
    }
    
    // Read encoders from right controller
    if (!roboclaw.ReadEncoders(addressRight, fr, rr)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::getAllSpeeds(int32_t& fl, int32_t& rl, int32_t& fr, int32_t& rr) {
    bool valid;
    
    // Read speeds from left controller
    rl = roboclaw.ReadSpeedM1(addressLeft, nullptr, &valid);
    if (!valid) return false;
    
    fl = roboclaw.ReadSpeedM2(addressLeft, nullptr, &valid);
    if (!valid) return false;
    
    // Read speeds from right controller
    fr = roboclaw.ReadSpeedM1(addressRight, nullptr, &valid);
    if (!valid) return false;
    
    rr = roboclaw.ReadSpeedM2(addressRight, nullptr, &valid);
    if (!valid) return false;
    
    return true;
}

bool RoboClawDriver::getAllCurrents(int16_t& fl, int16_t& rl, int16_t& fr, int16_t& rr) {
    bool success = true;
    
    // Read currents from left controller
    if (!roboclaw.ReadCurrents(addressLeft, rl, fl)) {
        success = false;
    }
    
    // Read currents from right controller
    if (!roboclaw.ReadCurrents(addressRight, fr, rr)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::getVoltages(uint16_t& left, uint16_t& right) {
    bool valid;
    
    // Read main battery voltage from left controller
    left = roboclaw.ReadMainBatteryVoltage(addressLeft, &valid);
    if (!valid) return false;
    
    // Read main battery voltage from right controller
    right = roboclaw.ReadMainBatteryVoltage(addressRight, &valid);
    if (!valid) return false;
    
    return true;
}

bool RoboClawDriver::getErrors(uint32_t& left, uint32_t& right) {
    bool valid;
    
    // Read error status from left controller
    left = roboclaw.ReadError(addressLeft, &valid);
    if (!valid) return false;
    
    // Read error status from right controller
    right = roboclaw.ReadError(addressRight, &valid);
    if (!valid) return false;
    
    return true;
}

bool RoboClawDriver::getTemperatures(uint16_t& left, uint16_t& right) {
    bool success = true;
    
    // Read temperature from left controller
    if (!roboclaw.ReadTemp(addressLeft, left)) {
        success = false;
    }
    
    // Read temperature from right controller
    if (!roboclaw.ReadTemp(addressRight, right)) {
        success = false;
    }
    
    return success;
}

bool RoboClawDriver::updateStatusArray(int64_t* statusArray) {
    // Read encoder positions
    int32_t encFL, encRL, encFR, encRR;
    if (!getAllEncoders(encFL, encRL, encFR, encRR)) {
        return false;
    }
    
    statusArray[EncFL] = encFL;
    statusArray[EncBL] = encRL;
    statusArray[EncFR] = encFR;
    statusArray[EncBR] = encRR;
    
    // Read wheel speeds
    int32_t spdFL, spdRL, spdFR, spdRR;
    if (!getAllSpeeds(spdFL, spdRL, spdFR, spdRR)) {
        return false;
    }
    
    statusArray[SpdFL] = spdFL;
    statusArray[SpdRL] = spdRL;
    statusArray[SpdFR] = spdFR;
    statusArray[SpdRR] = spdRR;
    
    // Read motor currents
    int16_t curFL, curRL, curFR, curRR;
    if (!getAllCurrents(curFL, curRL, curFR, curRR)) {
        return false;
    }
    
    statusArray[CurFL] = curFL;
    statusArray[CurBL] = curRL;
    statusArray[CurFR] = curFR;
    statusArray[CurBR] = curRR;
    
    // Read battery voltages
    uint16_t voltL, voltR;
    if (!getVoltages(voltL, voltR)) {
        return false;
    }
    
    statusArray[VolL] = voltL;
    statusArray[VolR] = voltR;
    
    // Read error status
    uint32_t errL, errR;
    if (!getErrors(errL, errR)) {
        return false;
    }
    
    statusArray[ErrL] = errL;
    statusArray[ErrR] = errR;
    
    // Read controller temperatures
    uint16_t tempL, tempR;
    if (!getTemperatures(tempL, tempR)) {
        return false;
    }
    
    statusArray[TempL] = tempL;
    statusArray[TempR] = tempR;
    
    // Set timestamp
    statusArray[Timestamp] = millis();
    
    return true;
}