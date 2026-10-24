//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-10-24
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module  : Base Station - Main Application: 
  Phase   : wifi serial debug, ESP-NOW transport, SD logging, RTC timestamp
*/

#include <Arduino.h>

#include "../common/weatherbus_protocol.h"

#include <Wire.h>
#include "../config/base/node_config.h"
#include "../services/base/polling_engine.h"
#include "../drivers/RTCClock.h"
#include "../services/base/LocalSensorService.h"
#include "../drivers/SHT41Source.h"
#include "../drivers/SDLogger.h"
#include "../interfaces/IDataLogger.h"
#include "../drivers/ESPNowTransport.h"
#include "../services/debug/WirelessDebugService.h"
#include "../services/debug/DebugSerial.h"

NodeManager nodeManager;
ESPNowTransport transport;
RTCClock rtcClock;
WirelessDebugService wirelessDebug;

PollingEngine pollingEngine(
    nodeManager,
    transport);

SHT41Source sht41Source;

LocalSensorService localSensorService(sht41Source);

// =====================================================
// SD MODULE
// =====================================================

constexpr uint8_t SD_CS_PIN = 10;

SDLogger sdLogger(SD_CS_PIN);
IDataLogger& dataLogger = sdLogger;

// =====================================================
// RTC TIMESTAMP
// =====================================================

bool getTimestamp(char* buffer, size_t bufferSize) {
    if (!rtcClock.isOK() || bufferSize < 20) {
        return false;
    }

    DateTime now = rtcClock.now();

    snprintf(buffer, bufferSize,
             "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    return true;
}

// =====================================================
// Node log pending/buffer
// =====================================================

volatile bool nodeLogPending = false;

uint8_t pendingLogNodeId = 0;
float pendingLogTemperature = 0.0f;
float pendingLogHumidity = 0.0f;
float pendingLogPressure = 0.0f;
uint8_t pendingLogFlags = 0;

// =====================================================
// Transport RX callback
//
// Transport hanya menyerahkan raw bytes.
// WeatherBus packet validation kekal di sini.
// =====================================================

void onTransportDataReceived(
    const uint8_t* data,
    size_t length) {
    if (length !=
        sizeof(WeatherBus::SensorDataPacket)) {

        Serial.println(
            "RX: Invalid packet size");

        return;
    }

    WeatherBus::SensorDataPacket packet{};

    memcpy(&packet, data, sizeof(packet));

    if (packet.header.protocolVersion !=
        WeatherBus::PROTOCOL_VERSION) {

        Serial.println(
            "RX: Invalid protocol version");

        return;
    }

    if (packet.header.packetType !=
        static_cast<uint8_t>(
            WeatherBus::PacketType::SENSOR_DATA)) {

        Serial.println(
            "RX: Unexpected packet type");

        return;
    }

    if (packet.header.payloadLength !=
        sizeof(WeatherBus::SensorDataPayload)) {

        Serial.println(
            "RX: Invalid payload length");

        return;
    }

    pollingEngine.onSensorData(
        packet.header.nodeId,
        packet.header.sequence,
        packet.header.flags,
        packet.payload.temperature,
        packet.payload.humidity,
        packet.payload.pressure);
}

// -------------------------------------------------
// Node SD logging
//
// Callback only store the  data.
// SD writing is done in the main loop to avoid blocking the callback.
// -------------------------------------------------

void onValidNodeResponse(
    uint8_t nodeId,
    uint8_t flags,
    float temperature,
    float humidity,
    float pressure) {

    pendingLogNodeId = nodeId;
    pendingLogTemperature = temperature;
    pendingLogHumidity = humidity;
    pendingLogPressure = pressure;
    pendingLogFlags = flags;

    nodeLogPending = true;
}

// =====================================================
// Add all Node peers
//
// Transport unknown the NodeManager.
// main .cpp is the only place that knows both.
// =====================================================

bool setupPeers() {
    for (uint8_t i = 0;
         i < nodeManager.getNodeCount();
         i++) {

        NodeInfo* node = nodeManager.getNode(i);

        if (node == nullptr) {
            continue;
        }
        TransportAddress address{};
        address.length = 6;

        memcpy(
            address.data,
            node->mac,
            address.length);

        if (!transport.addPeer(address)) {
            Serial.printf(
                "ERROR: Failed to add Node %u\n",
                node->nodeId);
            return false;
        }
        Serial.printf(
            "Peer added: Node %u\n",
            node->nodeId);
    }
    return true;
}

// =====================================================
// Transport initialization
// =====================================================

bool setupTransport() {
    if (!transport.begin(onTransportDataReceived)) {
        Serial.println(
            "ERROR: Transport initialization failed");
        return false;
    }
    if (!setupPeers()) {
        return false;
    }
    return true;
}

// =====================================================
// I2C initialization
// =====================================================
void setupI2C() {
    Wire.begin(8, 9);
    Wire.setClock(100000);
    delay(10);

    Serial.println("I2C bus ready");
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);

    if (!wirelessDebug.begin()) {
        Serial.println(
            "[WiFi Debug] Initialization failed");
    }

    setupI2C();

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.println("================================");
    Serial.println();

    nodeManager.begin();

    if (!localSensorService.begin()) {
        Serial.println(
            "ERROR: Local Sensor Service initialization failed");
    }

    if (!dataLogger.begin()) {
        Serial.println(
            F("[SD] Logger unavailable"));
    }

    // -------------------------------------------------
    // Initialize RTC
    // -------------------------------------------------

    if (rtcClock.begin()) {
        Serial.println(
            "RTC: BEGIN OK");

        // Set RTC using compile date/time ONE TIME ONLY
        // rtc.setDateTime(
        //     DateTime(F(__DATE__), F(__TIME__)));

        if (rtcClock.update()) {
            Serial.println(
                "RTC: UPDATE OK");
        } else {
            Serial.println(
                "RTC: UPDATE FAILED");
        }
    }

    // -------------------------------------------------
    // Initialize Transport
    // -------------------------------------------------

    if (!setupTransport()) {

        Serial.println(
            "SYSTEM HALTED");

        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println("Transport ready");

    pollingEngine.begin();
    pollingEngine.setValidResponseCallback(
        onValidNodeResponse);

    delay(5000);
}

// =====================================================
// Main loop
// =====================================================

void loop() {
    // -------------------------------------------------
    // Wireless Debug
    // -------------------------------------------------

    wirelessDebug.update();

    // -------------------------------------------------
    // Transport
    // -------------------------------------------------

    transport.update();

    // -------------------------------------------------
    // Polling Engine
    // -------------------------------------------------

    pollingEngine.update();

    // -------------------------------------------------
    // SD Logger
    // -------------------------------------------------

    sdLogger.update();

    // -------------------------------------------------
    // RTC
    // -------------------------------------------------

    static unsigned long lastUpdate = 0;

    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;

        static bool rtcInitialized = false;
        static bool rtcWasOK = false;

        bool rtcOK = rtcClock.update();

        if (!rtcInitialized) {
            // Bacaan pertama selepas boot
            if (rtcOK) {
                Serial.print(F("[RTC] Initial update OK; timestamp: "));

                DateTime now = rtcClock.now();

                Serial.printf(
                    "%04d-%02d-%02d %02d:%02d:%02d\n",
                    now.year(),
                    now.month(),
                    now.day(),
                    now.hour(),
                    now.minute(),
                    now.second());

            } else {
                Serial.println(F("[RTC] Initial update failed"));
            }

            rtcInitialized = true;
        } else if (!rtcOK) {
            Serial.println(F("[RTC] Update failed"));
        } else if (!rtcWasOK) {
            // RTC pulih selepas kegagalan sebelumnya
            Serial.print(F("[RTC] Update OK; timestamp: "));

            DateTime now = rtcClock.now();

            Serial.printf(
                "%04d-%02d-%02d %02d:%02d:%02d\n",
                now.year(),
                now.month(),
                now.day(),
                now.hour(),
                now.minute(),
                now.second());

            Serial.println(F("[RTC] Recovered"));
        }

        rtcWasOK = rtcOK;
    }
    /*
    if (currentMillis - lastUpdate >= 1000) {
        lastUpdate = currentMillis;

        if (!rtcClock.update()) {
            Serial.println(F("[RTC] Update failed"));
        }
    } */

    // =================================================
    // SD LOGGING - NODE
    // =================================================

    if (nodeLogPending) {
        nodeLogPending = false;
        char timestamp[20];

        if (getTimestamp(timestamp, sizeof(timestamp))) {
            if (dataLogger.logSensorData(
                    timestamp,
                    pendingLogNodeId,
                    pendingLogTemperature,
                    pendingLogHumidity,
                    pendingLogPressure,
                    pendingLogFlags)) {

                Serial.println(F("[SD] Node sensor logged"));
            }
        } else {
            Serial.println(F("[RTC] Invalid timestamp; Node logging skipped"));
        }
    }

    // =================================================
    // LOCAL SENSOR
    // =================================================

    static uint32_t lastSensorRead = 0;

    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead = millis();
        WeatherBus::SensorDataPayload data{};
        uint8_t flags = 0;
        //localSensorService.readSensors(data, flags);
        bool sensorReadOK =
            localSensorService.readSensors(data, flags);

        if (!sensorReadOK) {
            Serial.println("WARNING: Local sensor read failed");
        }

        Serial.println();
        Serial.println(
            "========== LOCAL SENSOR ==========");

        Serial.printf(
            "Flags : 0x%02X\n",
            flags);

        Serial.printf(
            "Temperature : %.2f C\n",
            data.temperature);

        Serial.printf(
            "Humidity    : %.2f %%\n",
            data.humidity);

        Serial.printf(
            "Pressure    : %.2f hPa\n",
            data.pressure);

        Serial.println(
            "==================================");
        // -------------------------------------------------
        // SD logging - Local Sensor
        // -------------------------------------------------

        if (sensorReadOK) {
            char timestamp[20];

            if (getTimestamp(timestamp, sizeof(timestamp))) {
                if (dataLogger.logSensorData(
                        timestamp,
                        0,
                        data.temperature,
                        data.humidity,
                        data.pressure,
                        flags)) {

                    Serial.println(F("[SD] Local sensor logged"));
                }
            } else {
                Serial.println(
                    F("[RTC] Invalid timestamp; local logging skipped"));
            }
        }
    }
}
