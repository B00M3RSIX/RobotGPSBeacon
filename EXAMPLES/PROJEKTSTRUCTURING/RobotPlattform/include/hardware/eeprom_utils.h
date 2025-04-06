#include <EEPROM.h>
#include "MPU9250.h"
#include "system/DebugUtils.h"

const uint8_t EEPROM_SIZE = 1 + sizeof(float) * 3 * 4;
extern MPU9250 mpu;

enum EEP_ADDR {
    EEP_CALIB_FLAG = 0x00,
    EEP_ACC_BIAS = 0x01,
    EEP_GYRO_BIAS = 0x0D,
    EEP_MAG_BIAS = 0x19,
    EEP_MAG_SCALE = 0x25
};

void writeByte(int address, byte value) {
    EEPROM.put(address, value);
}

void writeFloat(int address, float value) {
    EEPROM.put(address, value);
}

byte readByte(int address) {
    byte valueIn = 0;
    EEPROM.get(address, valueIn);
    return valueIn;
}

float readFloat(int address) {
    float valueIn = 0;
    EEPROM.get(address, valueIn);
    return valueIn;
}

void clearCalibration() {
    writeByte(EEP_CALIB_FLAG, 0);
}

bool isCalibrated() {
    return (readByte(EEP_CALIB_FLAG) == 0x01);
}

void saveCalibration() {
    DEBUG_PRINTLN("Write calibrated parameters to EEPROM");
    writeByte(EEP_CALIB_FLAG, 1);
    writeFloat(EEP_ACC_BIAS + 0, mpu.getAccBias(0));
    writeFloat(EEP_ACC_BIAS + 4, mpu.getAccBias(1));
    writeFloat(EEP_ACC_BIAS + 8, mpu.getAccBias(2));
    writeFloat(EEP_GYRO_BIAS + 0, mpu.getGyroBias(0));
    writeFloat(EEP_GYRO_BIAS + 4, mpu.getGyroBias(1));
    writeFloat(EEP_GYRO_BIAS + 8, mpu.getGyroBias(2));
    writeFloat(EEP_MAG_BIAS + 0, mpu.getMagBias(0));
    writeFloat(EEP_MAG_BIAS + 4, mpu.getMagBias(1));
    writeFloat(EEP_MAG_BIAS + 8, mpu.getMagBias(2));
    writeFloat(EEP_MAG_SCALE + 0, mpu.getMagScale(0));
    writeFloat(EEP_MAG_SCALE + 4, mpu.getMagScale(1));
    writeFloat(EEP_MAG_SCALE + 8, mpu.getMagScale(2));
#if defined(ESP_PLATFORM) || defined(ESP8266)
    EEPROM.commit();
#endif
}

void loadCalibration() {
    DEBUG_PRINTLN("Load calibrated parameters from EEPROM");
    if (isCalibrated()) {
        DEBUG_PRINTLN("calibrated? : YES");
        DEBUG_PRINTLN("load calibrated values");
        mpu.setAccBias(
            readFloat(EEP_ACC_BIAS + 0),
            readFloat(EEP_ACC_BIAS + 4),
            readFloat(EEP_ACC_BIAS + 8));
        mpu.setGyroBias(
            readFloat(EEP_GYRO_BIAS + 0),
            readFloat(EEP_GYRO_BIAS + 4),
            readFloat(EEP_GYRO_BIAS + 8));
        mpu.setMagBias(
            readFloat(EEP_MAG_BIAS + 0),
            readFloat(EEP_MAG_BIAS + 4),
            readFloat(EEP_MAG_BIAS + 8));
        mpu.setMagScale(
            readFloat(EEP_MAG_SCALE + 0),
            readFloat(EEP_MAG_SCALE + 4),
            readFloat(EEP_MAG_SCALE + 8));
    } else {
        DEBUG_PRINTLN("calibrated? : NO");
        DEBUG_PRINTLN("load default values");
        mpu.setAccBias(0., 0., 0.);
        mpu.setGyroBias(0., 0., 0.);
        mpu.setMagBias(0., 0., 0.);
        mpu.setMagScale(1., 1., 1.);
    }
}

void printCalibration() {
    DEBUG_PRINTLN("< calibration parameters >");
    DEBUG_PRINT("calibrated? : ");
    DEBUG_PRINTLN(readByte(EEP_CALIB_FLAG) ? "YES" : "NO");
    DEBUG_PRINT("acc bias x  : ");
    DEBUG_PRINTLN(readFloat(EEP_ACC_BIAS + 0) * 1000.f / MPU9250::CALIB_ACCEL_SENSITIVITY);
    DEBUG_PRINT("acc bias y  : ");
    DEBUG_PRINTLN(readFloat(EEP_ACC_BIAS + 4) * 1000.f / MPU9250::CALIB_ACCEL_SENSITIVITY);
    DEBUG_PRINT("acc bias z  : ");
    DEBUG_PRINTLN(readFloat(EEP_ACC_BIAS + 8) * 1000.f / MPU9250::CALIB_ACCEL_SENSITIVITY);
    DEBUG_PRINT("gyro bias x : ");
    DEBUG_PRINTLN(readFloat(EEP_GYRO_BIAS + 0) / MPU9250::CALIB_GYRO_SENSITIVITY);
    DEBUG_PRINT("gyro bias y : ");
    DEBUG_PRINTLN(readFloat(EEP_GYRO_BIAS + 4) / MPU9250::CALIB_GYRO_SENSITIVITY);
    DEBUG_PRINT("gyro bias z : ");
    DEBUG_PRINTLN(readFloat(EEP_GYRO_BIAS + 8) / MPU9250::CALIB_GYRO_SENSITIVITY);
    DEBUG_PRINT("mag bias x  : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_BIAS + 0));
    DEBUG_PRINT("mag bias y  : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_BIAS + 4));
    DEBUG_PRINT("mag bias z  : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_BIAS + 8));
    DEBUG_PRINT("mag scale x : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_SCALE + 0));
    DEBUG_PRINT("mag scale y : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_SCALE + 4));
    DEBUG_PRINT("mag scale z : ");
    DEBUG_PRINTLN(readFloat(EEP_MAG_SCALE + 8));
}

void printBytes() {
    for (size_t i = 0; i < EEPROM_SIZE; ++i)
        DEBUG_PRINTLN(readByte(i), HEX);
}

void setupEEPROM() {
    DEBUG_PRINTLN("EEPROM start");

    if (!isCalibrated()) {
        DEBUG_PRINTLN("Need Calibration!!");
    }
    DEBUG_PRINTLN("EEPROM calibration value is : ");
    printCalibration();
    DEBUG_PRINTLN("Loaded calibration value is : ");
    loadCalibration();
}
