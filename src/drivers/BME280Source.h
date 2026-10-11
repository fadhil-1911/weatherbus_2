// =====================================================
// File: src/drivers/BME280Source.h
// =====================================================
#pragma once

#include <Adafruit_BME280.h>

#include "../interfaces/ISensorSource.h"

class BME280Source : public ISensorSource {
  public:
    explicit BME280Source(
        uint8_t address = 0x76);

    bool begin() override;

    bool read(
        SensorReading& reading) override;

  private:
    bool recover();

    bool isTemperatureValid(
        float value) const;

    bool isHumidityValid(
        float value) const;

    bool isPressureValid(
        float value) const;

    Adafruit_BME280 _bme;
    uint8_t _address;
};
