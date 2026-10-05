#pragma once

#include <Arduino.h>

#include "../common/weatherbus_protocol.h"
#include "../interfaces/ISensorSource.h"

class LocalSensorService
{
public:
    explicit LocalSensorService(ISensorSource& sensorSource);

    bool begin();

    bool readSensors(
        WeatherBus::SensorDataPayload& data,
        uint8_t& flags);

private:
    ISensorSource& sensorSource;
};