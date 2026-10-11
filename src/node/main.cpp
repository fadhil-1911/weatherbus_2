//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-10-01
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module    : Sensor Node - Main Application
  Transport : ESP-NOW
  Phase     : 
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include "config/node_settings.h"
#include "../drivers/BME280Source.h"
#include "../drivers/SHT41Source.h"

#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

// =====================================================
// SENSOR DRIVER OBJECT
// =====================================================

SHT41Source sht41Source;
BME280Source bme280Source(BME280_I2C_ADDRESS);

// =====================================================
// Send SENSOR_DATA
// =====================================================

void sendSensorData(uint16_t requestSequence) {

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    uint8_t sensorFlags = 0;

    // -------------------------------------------------
    // Read BME280 through driver
    // -------------------------------------------------

    if (ENABLE_BME280) {
        SensorReading reading;

        if (bme280Source.read(reading)) {
            pressure = reading.pressure;

            if (reading.pressureValid) {
                sensorFlags |= FLAG_PRESSURE_VALID;
            }

            if (USE_BME280_TEMPERATURE &&
                reading.temperatureValid) {
                temperature = reading.temperature;
                sensorFlags |= FLAG_TEMPERATURE_VALID;
            }

            if (USE_BME280_HUMIDITY &&
                reading.humidityValid) {
                humidity = reading.humidity;
                sensorFlags |= FLAG_HUMIDITY_VALID;
            }
        } else {
            Serial.println("WARNING: BME280 read failed");
        }
    }

    // -------------------------------------------------
    // Read SHT41 through ISensorSource
    // -------------------------------------------------

    if (ENABLE_SHT41) {
        SensorReading shtReading;

        if (sht41Source.read(shtReading)) {

            // Use SHT41 temperature if selected
            if (!USE_BME280_TEMPERATURE &&
                shtReading.temperatureValid) {

                temperature = shtReading.temperature;
                sensorFlags |= FLAG_TEMPERATURE_VALID;
            }

            // Use SHT41 humidity if selected
            if (!USE_BME280_HUMIDITY &&
                shtReading.humidityValid) {

                humidity = shtReading.humidity;
                sensorFlags |= FLAG_HUMIDITY_VALID;
            }

        } else {
            Serial.println("WARNING: SHT41 read failed");
        }
    }

    // -------------------------------------------------
    // Build SENSOR_DATA packet
    // -------------------------------------------------

    SensorDataPacket packet{};

    packet.header.protocolVersion = PROTOCOL_VERSION;
    packet.header.packetType =
        static_cast<uint8_t>(
            PacketType::SENSOR_DATA);

    packet.header.nodeId = NODE_ID;
    packet.header.flags = sensorFlags;
    packet.header.sequence = requestSequence;
    packet.header.payloadLength = sizeof(SensorDataPayload);
    packet.header.crc16 = 0;
    packet.header.reserved = 0;
    packet.payload.temperature = temperature;
    packet.payload.humidity = humidity;
    packet.payload.pressure = pressure;

    // -------------------------------------------------
    // ESP-NOW transmission
    // -------------------------------------------------

    esp_err_t result = esp_now_send(
        baseMac,
        reinterpret_cast<uint8_t*>(&packet),
        sizeof(packet));

    if (result == ESP_OK) {

        Serial.printf(
            "TX: SENSOR_DATA | "
            "Node=%u | "
            "Seq=%u | "
            "Flags=0x%02X | "
            "T=%.2f C | "
            "RH=%.2f %% | "
            "P=%.2f hPa\n",

            NODE_ID,
            packet.header.sequence,
            packet.header.flags,
            packet.payload.temperature,
            packet.payload.humidity,
            packet.payload.pressure);
    } else {

        Serial.printf(
            "TX ERROR: %d\n",
            result);
    }
}

// =====================================================
// Receive DATA_REQUEST
// =====================================================

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len != sizeof(Header)) {
        Serial.printf(
            "RX ERROR: Invalid packet size | Received=%d | Expected=%u\n",
            len,
            sizeof(Header));

        return;
    }

    Header request{};

    memcpy(&request, data, sizeof(request));

    if (request.protocolVersion != PROTOCOL_VERSION) {
        Serial.printf(
            "RX ERROR: Invalid protocol version | Received=%u | Expected=%u\n",
            request.protocolVersion,
            PROTOCOL_VERSION);

        return;
    }

    if (request.packetType != static_cast<uint8_t>(PacketType::DATA_REQUEST)) {
        Serial.printf("RX ERROR: Invalid packet type | Received=0x%02X\n",
                      request.packetType);
        return;
    }

    if (request.nodeId != NODE_ID) {
        Serial.printf("RX ERROR: Invalid Node ID | Received=%u | This Node=%u\n",
                      request.nodeId,
                      NODE_ID);

        return;
    }

    if (request.payloadLength != 0) {
        Serial.printf("RX ERROR: Invalid payload length | Received=%u | Expected=0\n",
                      request.payloadLength);

        return;
    }

    Serial.printf("RX: DATA_REQUEST | Node=%u | Seq=%u\n",
                  NODE_ID,
                  request.sequence);

    sendSensorData(request.sequence);
}

// =====================================================
// ESP-NOW setup
// =====================================================

bool setupEspNow() {
    WiFi.mode(WIFI_STA);

    Serial.print("Node MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("ERROR: ESP-NOW initialization failed");
        return false;
    }

    esp_now_register_recv_cb(onDataReceived);

    esp_now_peer_info_t peerInfo{};

    memcpy(peerInfo.peer_addr, baseMac, 6);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("ERROR: Failed to add Base peer");
        return false;
    }

    return true;
}

bool setupI2C() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_CLOCK);

    delay(10);

    Serial.println("I2C bus ready");
    return true;
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!setupI2C()) {
        Serial.println("I2C initialization failed");

        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.printf("NODE ID: %u\n", NODE_ID);
    Serial.println("================================");

    Serial.println();
    Serial.println("Sensor Configuration:");

    Serial.printf(
        "SHT41  : %s\n",
        ENABLE_SHT41 ? "ENABLED" : "DISABLED");

    Serial.printf(
        "BME280 : %s\n",
        ENABLE_BME280 ? "ENABLED" : "DISABLED");

    Serial.println();

    if (ENABLE_SHT41) {
        if (!sht41Source.begin()) {
            Serial.println("SHT41 FAILED");
            Serial.println(
                "SHT41 unavailable - continuing without temperature/humidity");
        }
    }

    if (ENABLE_BME280) {
        if (!bme280Source.begin()) {
            Serial.println("BME280 FAILED");
            Serial.println("BME280 unavailable - continuing without pressure");
        }
    }

    if (!setupEspNow()) {
        Serial.println("SYSTEM HALTED");

        while (true) {
            delay(1000);
        }
    }

    Serial.println("ESP-NOW ready");
    Serial.println("Node ready");
}

// =====================================================
// Loop
// =====================================================

void loop() {
    // No polling required.
    //
    // Node responds only when a valid
    // DATA_REQUEST is received.
}
