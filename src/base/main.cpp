//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-09-30
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module  : Base Station - Main Application
  Transport : ESP-NOW
  Phase   : 
*/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "../common/weatherbus_protocol.h"
#include "node_manager.h"
#include "polling_engine.h"

#include "local_sensor_manager.h"

#include "SDLogger.h"

NodeManager nodeManager;
LocalSensorManager localSensorManager; // local sensor manager instance
PollingEngine pollingEngine(nodeManager);

// =====================================================
// SD MODULE
// =====================================================

constexpr uint8_t SD_CS_PIN = 10;
SDLogger sdLogger(SD_CS_PIN);

volatile bool nodeLogPending = false;

uint8_t pendingLogNodeId = 0;
float pendingLogTemperature = 0.0f;
float pendingLogHumidity = 0.0f;
float pendingLogPressure = 0.0f;
uint8_t pendingLogFlags = 0;

// =====================================================
// ESP-NOW RX callback
// =====================================================

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(WeatherBus::SensorDataPacket)) {
        Serial.println("RX: Invalid packet size");
        return;
    }
    WeatherBus::SensorDataPacket packet{};
    memcpy(&packet, data, sizeof(packet));
    if (packet.header.protocolVersion != WeatherBus::PROTOCOL_VERSION) {
        Serial.println("RX: Invalid protocol version");
        return;
    }
    if (packet.header.packetType != static_cast<uint8_t>(WeatherBus::PacketType::SENSOR_DATA)) {
        Serial.println("RX: Unexpected packet type");
        return;
    }
    if (packet.header.payloadLength != sizeof(WeatherBus::SensorDataPayload)) {
        Serial.println("RX: Invalid payload length");
        return;
    }
    pollingEngine.onSensorData(
        packet.header.nodeId,
        packet.header.sequence,
        packet.header.flags,
        packet.payload.temperature,
        packet.payload.humidity,
        packet.payload.pressure);

    // LOGGING TO SD CARD
    pendingLogNodeId = packet.header.nodeId;
    pendingLogTemperature = packet.payload.temperature;
    pendingLogHumidity = packet.payload.humidity;
    pendingLogPressure = packet.payload.pressure;
    pendingLogFlags = packet.header.flags;

    nodeLogPending = true;
}

// =====================================================
// Add all Node peers
// =====================================================

bool setupPeers() {
    for (uint8_t i = 0; i < nodeManager.getNodeCount(); i++) {
        NodeInfo* node = nodeManager.getNode(i);
        if (node == nullptr) {
            continue;
        }

        esp_now_peer_info_t peerInfo{};
        memcpy(peerInfo.peer_addr, node->mac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        esp_err_t result = esp_now_add_peer(&peerInfo);
        if (result != ESP_OK) {
            Serial.printf("ERROR: Failed to add Node %u | %d\n", node->nodeId, result);
            return false;
        }
        Serial.printf("Peer added: Node %u\n", node->nodeId);
    }
    return true;
}

// =====================================================
// ESP-NOW initialization
// =====================================================

bool setupEspNow() {
    WiFi.mode(WIFI_STA);
    Serial.print("Base MAC: ");
    Serial.println(WiFi.macAddress());
    if (esp_now_init() != ESP_OK) {
        Serial.println("ERROR: ESP-NOW initialization failed");
        return false;
    }
    esp_now_register_recv_cb(onDataReceived);
    return setupPeers();
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.println("================================");
    Serial.println();
    nodeManager.begin();
    localSensorManager.begin(); // Initialize the local sensor manager

    if (!sdLogger.begin()) {
        Serial.println(F("[SD] Logger unavailable"));
    }

    if (!setupEspNow()) {
        Serial.println("SYSTEM HALTED");
        while (true) {
            delay(1000);
        }
    }
    Serial.println();
    Serial.println("ESP-NOW ready");
    pollingEngine.begin();
}

// =====================================================
// Main loop
// =====================================================

void loop() {
    pollingEngine.update();

    if (nodeLogPending) {
        nodeLogPending = false;

        char timestamp[20];

        snprintf(
            timestamp,
            sizeof(timestamp),
            "UPTIME_%lu",
            millis());

        if (sdLogger.logSensorData(
                timestamp,
                pendingLogNodeId,
                pendingLogTemperature,
                pendingLogHumidity,
                pendingLogPressure,
                pendingLogFlags)) {
            Serial.println(F("[SD] Node sensor logged"));
        }
    }

    static uint32_t lastSensorRead = 0;

    if (millis() - lastSensorRead >= 2000) {
        lastSensorRead = millis();
        WeatherBus::SensorDataPayload data{};
        uint8_t flags = 0;
        localSensorManager.readSensors(data, flags);

        Serial.println();
        Serial.println("========== LOCAL SENSOR ==========");
        Serial.printf("Flags : 0x%02X\n", flags);
        Serial.printf("Temperature : %.2f C\n", data.temperature);
        Serial.printf("Humidity    : %.2f %%\n", data.humidity);
        Serial.printf("Pressure    : %.2f hPa\n", data.pressure);
        Serial.println("==================================");

        // SD logging
        char timestamp[20];

        snprintf(
            timestamp,
            sizeof(timestamp),
            "UPTIME_%lu",
            millis());

        if (sdLogger.logSensorData(
                timestamp,
                0,
                data.temperature,
                data.humidity,
                data.pressure,
                flags)) {
            Serial.println(F("[SD] Local sensor logged"));
        }
    }
}