// =====================================================
// File: src/drivers/BME280Source.cpp
// =====================================================

#include "BME280Source.h"
#include "../services/debug/DebugSerial.h"

#include <Wire.h>
#include <math.h>

// BME280 calibration offsets
namespace {
constexpr float TEMP_OFFSET = -0.68F;
constexpr float HUM_OFFSET = +8.98F;
}

BME280Source::BME280Source(uint8_t address)
    : _address(address) {
}

bool BME280Source::begin() {
    if (!_bme.begin(_address, &Wire)) {
        Serial.println(
            "ERROR: BME280 communication failed");
        return false;
    }

    Serial.println("BME280 communication OK");
    return true;
}

bool BME280Source::recover() {
    Serial.println("BME280 recovery attempt...");

    if (!_bme.begin(_address, &Wire)) {
        Serial.println("BME280 recovery failed");
        return false;
    }

    Serial.println("BME280 recovery OK");
    return true;
}

bool BME280Source::isTemperatureValid(
    float value) const {
    return isfinite(value) &&
           value > -40.0F &&
           value < 85.0F;
}

bool BME280Source::isHumidityValid(
    float value) const {
    return isfinite(value) &&
           value >= 0.0F &&
           value <= 100.0F;
}

bool BME280Source::isPressureValid(
    float value) const {
    return isfinite(value) &&
           value >= 300.0F &&
           value <= 1100.0F;
}

bool BME280Source::read(
    SensorReading& reading) {

    reading = SensorReading{};

    float temperature = _bme.readTemperature();
    float humidity = _bme.readHumidity();
    float pressure = _bme.readPressure() / 100.0F;

    // TEST T1: Keep temperature invalid
    //temperature = NAN;
    // TEST T2: Keep humidity invalid
    //humidity = NAN;
    // TEST T3: Keep pressure invalid
    //pressure = NAN;
    // TEST T4: Simulate complete sensor reading failure
    //temperature = NAN;
    //humidity = NAN;
    //pressure = NAN;

    Serial.printf(
        "BME280 RAW | T=%.2f C | RH=%.2f %% | P=%.2f hPa\n",
        temperature,
        humidity,
        pressure);

    bool temperatureValid =
        isTemperatureValid(temperature);

    bool humidityValid =
        isHumidityValid(humidity);

    bool pressureValid =
        isPressureValid(pressure);

    if (!temperatureValid ||
        !humidityValid ||
        !pressureValid) {

        Serial.println(
            "ERROR: BME280 reading invalid");

        if (!recover()) {
            return false;
        }

        temperature = _bme.readTemperature();
        humidity = _bme.readHumidity();
        pressure = _bme.readPressure() / 100.0F;

        // TEST T1: Keep temperature invalid
        //temperature = NAN;
        // TEST T2: Keep humidity invalid
        //humidity = NAN;
        // TEST T3: Keep pressure invalid
        //pressure = NAN;
        // TEST T4: Simulate complete sensor reading failure
        //temperature = NAN;
        //humidity = NAN;
        //pressure = NAN;

        temperatureValid =
            isTemperatureValid(temperature);

        humidityValid =
            isHumidityValid(humidity);

        pressureValid =
            isPressureValid(pressure);

        if (!temperatureValid &&
            !humidityValid &&
            !pressureValid) {

            Serial.println(
                "ERROR: All BME280 readings invalid after recovery");

            return false;
        }
        /*
        if (!temperatureValid ||
            !humidityValid ||
            !pressureValid) {

            Serial.println(
                "ERROR: BME280 readings still invalid after recovery");

            return false;
        } */
    }

    // Apply calibration only to valid readings
    if (temperatureValid) {
        temperature += TEMP_OFFSET;
    }

    if (humidityValid) {
        humidity += HUM_OFFSET;
        humidity = constrain(humidity, 0.0F, 100.0F);
    }

    // Store sensor readings
    reading.temperature = temperatureValid ? temperature : 0.0F;
    reading.humidity = humidityValid ? humidity : 0.0F;
    reading.pressure = pressureValid ? pressure : 0.0F;

    // Store validity status
    reading.temperatureValid = temperatureValid;
    reading.humidityValid = humidityValid;
    reading.pressureValid = pressureValid;

    return true;
}
