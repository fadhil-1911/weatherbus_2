#include "SHT41Source.h"

#include <Wire.h>

// =====================================================
// SHT41 CONFIGURATION
// =====================================================

namespace
{
    constexpr uint8_t SHT41_ADDRESS = 0x44;
    constexpr uint8_t SHT41_SDA = 8;
    constexpr uint8_t SHT41_SCL = 9;

    constexpr uint8_t SHT41_MEASURE_HIGH_PRECISION = 0xFD;
}

// =====================================================
// Constructor
// =====================================================

SHT41Source::SHT41Source()
    : available(false)
{
}

// =====================================================
// SHT41 CRC-8
// Polynomial: 0x31
// =====================================================

uint8_t SHT41Source::sht41Crc8(
    const uint8_t* data,
    uint8_t length)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// =====================================================
// Setup SHT41
// =====================================================

bool SHT41Source::setupSHT41()
{
    Wire.begin(SHT41_SDA, SHT41_SCL);
    Wire.setClock(100000);

    delay(10);

    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0)
    {
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
// Begin
// =====================================================

bool SHT41Source::begin()
{
    Serial.println("SHT41 Source");

    available = setupSHT41();

    if (!available)
    {
        Serial.println(
            "SHT41 unavailable - continuing");
    }

    return available;
}

// =====================================================
// Read SHT41
// =====================================================

bool SHT41Source::readSHT41(
    float& temperature,
    float& humidity)
{
    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0)
    {
        Serial.println(
            "ERROR: Local SHT41 measurement command failed");

        return false;
    }

    delay(10);

    uint8_t received =
        Wire.requestFrom(
            SHT41_ADDRESS,
            (uint8_t)6);

    if (received != 6)
    {
        Serial.println(
            "ERROR: Local SHT41 invalid response length");

        return false;
    }

    uint8_t data[6];

    for (uint8_t i = 0; i < 6; i++)
    {
        data[i] = Wire.read();
    }

    // -------------------------------------------------
    // Temperature CRC
    // -------------------------------------------------

    if (sht41Crc8(data, 2) != data[2])
    {
        Serial.println(
            "ERROR: Local SHT41 temperature CRC failed");

        return false;
    }

    // -------------------------------------------------
    // Humidity CRC
    // -------------------------------------------------

    if (sht41Crc8(&data[3], 2) != data[5])
    {
        Serial.println(
            "ERROR: Local SHT41 humidity CRC failed");

        return false;
    }

    // -------------------------------------------------
    // Raw values
    // -------------------------------------------------

    uint16_t rawTemperature =
        ((uint16_t)data[0] << 8) | data[1];

    uint16_t rawHumidity =
        ((uint16_t)data[3] << 8) | data[4];

    // -------------------------------------------------
    // Convert
    // -------------------------------------------------

    temperature =
        -45.0f +
        175.0f *
        ((float)rawTemperature / 65535.0f);

    humidity =
        -6.0f +
        125.0f *
        ((float)rawHumidity / 65535.0f);

    // -------------------------------------------------
    // Clamp humidity
    // -------------------------------------------------

    if (humidity < 0.0f)
    {
        humidity = 0.0f;
    }

    if (humidity > 100.0f)
    {
        humidity = 100.0f;
    }

    return true;
}

// =====================================================
// Read
// =====================================================

bool SHT41Source::read(
    SensorReading& reading)
{
    reading.temperature = 0.0f;
    reading.humidity = 0.0f;

    reading.temperatureValid = false;
    reading.humidityValid = false;

    if (!available)
    {
        return false;
    }

    if (!readSHT41(
            reading.temperature,
            reading.humidity))
    {
        return false;
    }

    reading.temperatureValid = true;
    reading.humidityValid = true;

    return true;
}