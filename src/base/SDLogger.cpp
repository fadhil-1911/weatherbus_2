
#include "SDLogger.h"

SDLogger::SDLogger(uint8_t csPin)
    : csPin(csPin),
      sdStatus(false) {
}

bool SDLogger::begin() {
    Serial.println(F("[SD] Initializing..."));

    // SPI pins:
    // SCK  = GPIO 4
    // MISO = GPIO 5
    // MOSI = GPIO 6
    // CS   = GPIO 10
    SPI.begin(4, 5, 6, 10);

    if (!sd.begin(csPin, SD_SCK_MHZ(10))) {
        Serial.println(F("[SD] Initialization failed"));

        sdStatus = false;

        return false;
    }

    sdStatus = true;

    Serial.println(F("[SD] Card OK"));

    // Create log.csv and CSV header if the file does not exist.
    if (!sd.exists(LOG_FILE)) {
        return createLogFile();
    }

    Serial.println(F("[SD] log_wb2.csv found"));

    return true;
}

bool SDLogger::createLogFile() {
    logFile = sd.open(LOG_FILE, FILE_WRITE);

    if (!logFile) {
        Serial.println(F("[SD] Failed to create log_wb2.csv"));

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

    logFile.close();

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
        Serial.println(F("[SD] Failed to open log.csv"));
        Serial.println(F("[SD] Logging disabled until reset"));

        sdStatus = false;

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

    logFile.close();

    return true;
}

bool SDLogger::isReady() const {
    return sdStatus;
}
