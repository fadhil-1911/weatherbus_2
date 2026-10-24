// =====================================================
// File: src/base/SDLogger.cpp
// =====================================================

#include "SDLogger.h"
#include "../services/debug/DebugSerial.h"

SDLogger::SDLogger(uint8_t csPin)
    : csPin(csPin),
      sdStatus(false),
      lastRetryMs(0) {
}

bool SDLogger::begin() {

    Serial.println(F("[SD] Initializing..."));

    sdStatus = false;

    // Ensure previous file handle is closed
    if (logFile) {
        logFile.close();
    }

    SPI.begin(4, 5, 6, 10);

    if (!sd.begin(csPin, SD_SCK_MHZ(10))) {
        Serial.println(F("[SD] Initialization failed"));
        return false;
    }

    Serial.println(F("[SD] Card OK"));

    // Create CSV if missing
    if (!sd.exists(LOG_FILE)) {
        if (!createLogFile()) {
            return false;
        }
    } else {
        Serial.print(F("[SD] "));
        Serial.print(LOG_FILE);
        Serial.println(F(" found"));
    }

    sdStatus = true;

    return true;
}

void SDLogger::update() {

    // SD is healthy, no recovery needed
    if (sdStatus) {
        return;
    }

    uint32_t now = millis();

    // Wait 5 seconds between retries
    if (now - lastRetryMs < RETRY_INTERVAL_MS) {
        return;
    }

    lastRetryMs = now;

    Serial.println(F("[SD] Attempting recovery..."));

    if (begin()) {
        Serial.println(F("[SD] Recovery successful"));
    } else {
        Serial.println(F("[SD] Recovery failed"));
    }
}

bool SDLogger::createLogFile() {
    logFile = sd.open(LOG_FILE, FILE_WRITE);

    if (!logFile) {
        Serial.println(F("[SD] Failed to create CSV"));
        sdStatus = false;
        return false;
    }

    logFile.println(
        "Timestamp,"
        "Node_ID,"
        "Temperature_C,"
        "Humidity_PCT,"
        "Pressure_hPa,"
        "Flags");

    bool writeOK = !logFile.getWriteError();
    bool syncOK = logFile.sync();
    logFile.close();

    if (!writeOK || !syncOK) {
        Serial.println(F("[SD] CSV header write failed"));
        sdStatus = false;
        return false;
    }

    Serial.println(F("[SD] CSV header created"));
    return true;
}

bool SDLogger::logSensorData(
    const char* timestamp,
    uint8_t nodeId,
    float temperature,
    float humidity,
    float pressure,
    uint8_t flags) {

    if (!sdStatus) {
        return false;
    }

    logFile = sd.open(LOG_FILE, FILE_WRITE);

    if (!logFile) {
        Serial.print(F("[SD] Failed to open "));
        Serial.println(LOG_FILE);
        Serial.println(F("[SD] Logging paused, waiting for recovery"));

        sdStatus = false;
        lastRetryMs = millis();
        return false;
    }

    // Timestamp
    logFile.print(timestamp);
    logFile.print(",");

    // Node ID
    logFile.print(nodeId);
    logFile.print(",");

    // Temperature
    if (flags & 0x01) {
        logFile.print(temperature, 2);
    } else {
        logFile.print("NA");
    }

    logFile.print(",");

    // Humidity
    if (flags & 0x02) {
        logFile.print(humidity, 2);
    } else {
        logFile.print("NA");
    }

    logFile.print(",");

    // Pressure
    if (flags & 0x04) {
        logFile.print(pressure, 2);
    } else {
        logFile.print("NA");
    }

    logFile.print(",");

    // WeatherBus sensor flags
    logFile.print("0x");

    if (flags < 0x10) {
        logFile.print("0");
    }

    logFile.println(flags, HEX);

    // Verify write operation
    bool writeOK = !logFile.getWriteError();

    // Flush data to SD
    bool syncOK = logFile.sync();

    // Close file
    logFile.close();

    if (!writeOK || !syncOK) {
        Serial.println(F("[SD] CSV write/sync failed"));

        sdStatus = false;
        lastRetryMs = millis();
        return false;
    }

    return true;
}

bool SDLogger::isReady() const {
    return sdStatus;
}
