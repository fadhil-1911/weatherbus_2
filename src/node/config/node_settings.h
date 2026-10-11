// =====================================================
// File: src/node/config/node_settings.h
// =====================================================

#pragma once

#include <Arduino.h>

// =====================================================
// NODE CONFIGURATION
// =====================================================

constexpr uint8_t NODE_ID = 1;

// =====================================================
// I2C CONFIGURATION
// =====================================================

constexpr uint8_t I2C_SDA = 8;
constexpr uint8_t I2C_SCL = 9;
constexpr uint32_t I2C_CLOCK = 100000;

// =====================================================
// SENSOR CONFIGURATION
// =====================================================

constexpr bool ENABLE_SHT41 = false;
constexpr bool ENABLE_BME280 = true;

constexpr bool USE_BME280_TEMPERATURE = true;
constexpr bool USE_BME280_HUMIDITY = true;

// =====================================================
// SHT41 CONFIGURATION
// =====================================================

constexpr uint8_t SHT41_ADDRESS = 0x44;
constexpr uint8_t SHT41_MEASURE_HIGH_PRECISION = 0xFD;

// =====================================================
// BME280 CONFIGURATION
// =====================================================

constexpr uint8_t BME280_I2C_ADDRESS = 0x76;

// =====================================================
// BASE MAC ADDRESS
// =====================================================

constexpr uint8_t baseMac[] = {
    0x0C, 0x4E, 0xA0,
    0x4D, 0x7C, 0x40
};
