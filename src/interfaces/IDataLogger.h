// =====================================================
// File: src/interfaces/IDataLogger.h
// =====================================================

#pragma once

#include <Arduino.h>

class IDataLogger {
  public:
    virtual ~IDataLogger() = default;

    virtual bool begin() = 0;

    virtual bool isReady() const = 0;

    virtual bool logSensorData(
        const char* timestamp,
        uint8_t nodeId,
        float temperature,
        float humidity,
        float pressure,
        uint8_t flags) = 0;
};
