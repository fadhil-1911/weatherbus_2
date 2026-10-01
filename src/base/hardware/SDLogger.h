// =====================================================
// File: src/base/SDLogger.h
// =====================================================

#pragma once

#include <Arduino.h>
#include <SdFat.h>

class SDLogger
{
public:
    explicit SDLogger(uint8_t csPin);

    bool begin();
    bool isReady() const;

    bool logSensorData(
        const char* timestamp,
        uint8_t nodeId,
        float temperature,
        float humidity,
        float pressure,
        uint8_t flags);

private:
    static constexpr const char* LOG_FILE = "log_wb2.csv";

    SdFat sd;
    FsFile logFile;
    uint8_t csPin;
    bool sdStatus;
    bool createLogFile();
};

