#pragma once

#include <Arduino.h>
#include "../common/weatherbus_protocol.h"

class LocalSensorManager {

public:
    LocalSensorManager();

    bool begin();

    bool readSensors(
        WeatherBus::SensorDataPayload& data,
        uint8_t& flags);

    bool readSHT41(
        float& temperature,
        float& humidity);

    bool readBME280(
        float& pressure);

private:

    bool setupSHT41();
    bool setupBME280();
    bool recoverBME280();

    uint8_t sht41Crc8(
        const uint8_t* data,
        uint8_t length);

    bool sht41Available;
    bool bme280Available;
};