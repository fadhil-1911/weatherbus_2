#include "LocalSensorService.h"

// =====================================================
// Constructor
// =====================================================

LocalSensorService::LocalSensorService(
    ISensorSource& sensorSource)
    : sensorSource(sensorSource)
{
}

// =====================================================
// Begin
// =====================================================

bool LocalSensorService::begin()
{
    return sensorSource.begin();
}

// =====================================================
// Read Sensors
// =====================================================

bool LocalSensorService::readSensors(
    WeatherBus::SensorDataPayload& data,
    uint8_t& flags)
{
    SensorReading reading;

    data.temperature = 0.0f;
    data.humidity = 0.0f;
    data.pressure = 0.0f;

    flags = 0;

    if (!sensorSource.read(reading))
    {
        return false;
    }

    // -------------------------------------------------
    // Temperature
    // -------------------------------------------------

    if (reading.temperatureValid)
    {
        data.temperature = reading.temperature;

        flags |=
            WeatherBus::FLAG_TEMPERATURE_VALID;
    }

    // -------------------------------------------------
    // Humidity
    // -------------------------------------------------

    if (reading.humidityValid)
    {
        data.humidity = reading.humidity;

        flags |=
            WeatherBus::FLAG_HUMIDITY_VALID;
    }

    // -------------------------------------------------
    // Pressure
    // -------------------------------------------------

    if (reading.pressureValid)
    {
        data.pressure = reading.pressure;

        flags |=
            WeatherBus::FLAG_PRESSURE_VALID;
    }

    return flags != 0;
}