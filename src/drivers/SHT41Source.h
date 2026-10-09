// =====================================================
// File: src/drivers/SHT41Source.h
// =====================================================

#pragma once

#include <Arduino.h>
#include "../interfaces/ISensorSource.h"

class SHT41Source : public ISensorSource {
  public:
    SHT41Source();

    bool begin() override;
    bool read(SensorReading& reading) override;

  private:
    bool setupSHT41();

    bool readSHT41(
        float& temperature,
        float& humidity);

    uint8_t sht41Crc8(
        const uint8_t* data,
        uint8_t length);

  private:
    bool available;
};