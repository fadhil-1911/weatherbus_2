#include "local_sensor_manager.h"

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// =====================================================
// SENSOR CONFIGURATION
// =====================================================

constexpr bool ENABLE_SHT41 = true;
constexpr bool ENABLE_BME280 = false;

// =====================================================
// SHT41 CONFIGURATION
// =====================================================

constexpr uint8_t SHT41_ADDRESS = 0x44;
constexpr uint8_t SHT41_SDA = 8;
constexpr uint8_t SHT41_SCL = 9;

constexpr uint8_t SHT41_MEASURE_HIGH_PRECISION = 0xFD;

// =====================================================
// BME280 CONFIGURATION
// =====================================================

constexpr uint8_t BME280_I2C_ADDRESS = 0x76;

Adafruit_BME280 bme;

// =====================================================
// Constructor
// =====================================================

LocalSensorManager::LocalSensorManager()
    : sht41Available(false),
      bme280Available(false) {
}

// =====================================================
// SHT41 CRC-8
// Polynomial: 0x31
// =====================================================

uint8_t LocalSensorManager::sht41Crc8(
    const uint8_t* data,
    uint8_t length) {

    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++) {

        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {

            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// =====================================================
// Setup SHT41
// =====================================================

bool LocalSensorManager::setupSHT41() {

    Wire.begin(
        SHT41_SDA,
        SHT41_SCL);

    Wire.setClock(100000);

    delay(10);

    Wire.beginTransmission(
        SHT41_ADDRESS);

    Wire.write(
        SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {

        Serial.println(
            "ERROR: Local SHT41 communication failed");

        return false;
    }

    delay(10);

    Serial.println(
        "Local SHT41 communication OK");

    return true;
}

// =====================================================
// Read SHT41
// =====================================================

bool LocalSensorManager::readSHT41(
    float& temperature,
    float& humidity) {

    Wire.beginTransmission(
        SHT41_ADDRESS);

    Wire.write(
        SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {

        Serial.println(
            "ERROR: Local SHT41 measurement command failed");

        return false;
    }

    delay(10);

    uint8_t received =
        Wire.requestFrom(
            SHT41_ADDRESS,
            (uint8_t)6);

    if (received != 6) {

        Serial.println(
            "ERROR: Local SHT41 invalid response length");

        return false;
    }

    uint8_t data[6];

    for (uint8_t i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }

    // Temperature CRC
    if (sht41Crc8(data, 2) != data[2]) {

        Serial.println(
            "ERROR: Local SHT41 temperature CRC failed");

        return false;
    }

    // Humidity CRC
    if (sht41Crc8(&data[3], 2) != data[5]) {

        Serial.println(
            "ERROR: Local SHT41 humidity CRC failed");

        return false;
    }

    uint16_t rawTemperature =
        ((uint16_t)data[0] << 8) |
        data[1];

    uint16_t rawHumidity =
        ((uint16_t)data[3] << 8) |
        data[4];

    temperature =
        -45.0f +
        175.0f *
        ((float)rawTemperature / 65535.0f);

    humidity =
        -6.0f +
        125.0f *
        ((float)rawHumidity / 65535.0f);

    if (humidity < 0.0f) {
        humidity = 0.0f;
    }

    if (humidity > 100.0f) {
        humidity = 100.0f;
    }

    return true;
}

// =====================================================
// Setup BME280
// =====================================================

bool LocalSensorManager::setupBME280() {

    if (!bme.begin(
            BME280_I2C_ADDRESS,
            &Wire)) {

        Serial.println(
            "ERROR: Local BME280 communication failed");

        return false;
    }

    Serial.println(
        "Local BME280 communication OK");

    return true;
}

// =====================================================
// BME280 recovery
// =====================================================

bool LocalSensorManager::recoverBME280() {

    Serial.println(
        "Local BME280 recovery attempt...");

    if (!bme.begin(
            BME280_I2C_ADDRESS,
            &Wire)) {

        Serial.println(
            "Local BME280 recovery failed");

        return false;
    }

    Serial.println(
        "Local BME280 recovery OK");

    return true;
}

// =====================================================
// Read BME280
// =====================================================

bool LocalSensorManager::readBME280(
    float& pressure) {

    pressure = 0.0f;

    float reading =
        bme.readPressure() / 100.0F;

    if (!isfinite(reading) ||
        reading < 300.0F ||
        reading > 1100.0F) {

        Serial.println(
            "ERROR: Local BME280 pressure invalid");

        if (!recoverBME280()) {
            return false;
        }

        reading =
            bme.readPressure() / 100.0F;

        if (!isfinite(reading) ||
            reading < 300.0F ||
            reading > 1100.0F) {

            Serial.println(
                "ERROR: Local BME280 pressure still invalid after recovery");

            return false;
        }
    }

    pressure = reading;

    return true;
}

// =====================================================
// Begin
// =====================================================

bool LocalSensorManager::begin() {

    Serial.println();
    Serial.println(
        "Local Sensor Manager");
    Serial.println(
        "--------------------");

    Serial.printf(
        "SHT41  : %s\n",
        ENABLE_SHT41 ? "ENABLED" : "DISABLED");

    Serial.printf(
        "BME280 : %s\n",
        ENABLE_BME280 ? "ENABLED" : "DISABLED");

    Serial.println(
        "I2C SDA: GPIO 8");

    Serial.println(
        "I2C SCL: GPIO 9");

    Serial.println();

    // -------------------------------------------------
    // Initialize I2C
    // -------------------------------------------------

    Wire.begin(
        SHT41_SDA,
        SHT41_SCL);

    Wire.setClock(100000);

    // -------------------------------------------------
    // SHT41
    // -------------------------------------------------

    if (ENABLE_SHT41) {

        sht41Available =
            setupSHT41();

        if (!sht41Available) {

            Serial.println(
                "SHT41 unavailable - continuing");
        }
    }

    // -------------------------------------------------
    // BME280
    // -------------------------------------------------

    if (ENABLE_BME280) {

        bme280Available =
            setupBME280();

        if (!bme280Available) {

            Serial.println(
                "BME280 unavailable - continuing");
        }
    }

    Serial.println();

    return true;
}

// =====================================================
// Read all local sensors
// =====================================================

bool LocalSensorManager::readSensors(
    WeatherBus::SensorDataPayload& data,
    uint8_t& flags) {

    data.temperature = 0.0f;
    data.humidity = 0.0f;
    data.pressure = 0.0f;

    flags = 0;

    // -------------------------------------------------
    // SHT41
    // -------------------------------------------------

    if (ENABLE_SHT41 &&
        sht41Available) {

        if (readSHT41(
                data.temperature,
                data.humidity)) {

            flags |=
                WeatherBus::FLAG_TEMPERATURE_VALID;

            flags |=
                WeatherBus::FLAG_HUMIDITY_VALID;

        } else {

            Serial.println(
                "WARNING: Local SHT41 read failed");
        }
    }

    // -------------------------------------------------
    // BME280
    // -------------------------------------------------

    if (ENABLE_BME280 &&
        bme280Available) {

        if (readBME280(
                data.pressure)) {

            flags |=
                WeatherBus::FLAG_PRESSURE_VALID;

        } else {

            Serial.println(
                "WARNING: Local BME280 read failed");
        }
    }

    return flags != 0;
}