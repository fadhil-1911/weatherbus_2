// =====================================================
// File: src/base/SDLogger.h
// =====================================================
#pragma once

#include <Arduino.h>
#include <SdFat.h>
#include "../interfaces/IDataLogger.h"

class SDLogger : public IDataLogger {

  public:
    explicit SDLogger(uint8_t csPin);

    bool begin() override;
    void update();

    bool isReady() const override;

    bool logSensorData(
        const char* timestamp,
        uint8_t nodeId,
        float temperature,
        float humidity,
        float pressure,
        uint8_t flags) override;

  private:
    static constexpr const char* LOG_FILE = "log_wb2.csv";
    static constexpr uint32_t RETRY_INTERVAL_MS = 5000;

    SdFat sd;
    FsFile logFile;

    uint8_t csPin;
    bool sdStatus;

    uint32_t lastRetryMs = 0;

    bool createLogFile();
};


