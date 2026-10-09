// =====================================================
// File: src/interfaces/ISensorSource.h
// =====================================================

#pragma once

#include <Arduino.h>

struct SensorReading {
    float temperature;
    float humidity;
    float pressure;

    bool temperatureValid;
    bool humidityValid;
    bool pressureValid;

    SensorReading()
        : temperature(0.0f),
          humidity(0.0f),
          pressure(0.0f),
          temperatureValid(false),
          humidityValid(false),
          pressureValid(false) {
    }
};

class ISensorSource {
  public:
    virtual ~ISensorSource() = default;

    virtual bool begin() = 0;

    virtual bool read(
        SensorReading& reading) = 0;
};