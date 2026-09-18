//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-09-14
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module    : Sensor Node - Main Application
  Transport : ESP-NOW
  Phase     : Phase 2C - BME280 integration
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

// =====================================================
// NODE CONFIGURATION
// =====================================================

constexpr uint8_t NODE_ID = 1;

// =====================================================
// SENSOR CONFIGURATION
// =====================================================

constexpr bool ENABLE_SHT41 = false;
constexpr bool ENABLE_BME280 = true;

// Select temperature and humidity source
constexpr bool USE_BME280_TEMPERATURE = true;
constexpr bool USE_BME280_HUMIDITY = true;

// =====================================================
// SHT41 CONFIGURATION
// =====================================================

constexpr uint8_t SHT41_ADDRESS = 0x44;
constexpr uint8_t SHT41_SDA = 8;
constexpr uint8_t SHT41_SCL = 9;

constexpr uint8_t SHT41_MEASURE_HIGH_PRECISION = 0xFD;

// =====================================================
// BME280 CONFIGURATION
// =====================================================

constexpr uint8_t BME280_I2C_ADDRESS = 0x76;

Adafruit_BME280 bme;

// =====================================================
// Base MAC
// =====================================================

uint8_t baseMac[] = {
    0x0C, 0x4E, 0xA0,
    0x4D, 0x7C, 0x40};

// =====================================================
// SHT41 CRC-8
// Polynomial: 0x31
// =====================================================

uint8_t sht41Crc8(const uint8_t* data, uint8_t length) {
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

// =====================================================
// Setup SHT41
// =====================================================

bool setupSHT41() {
    Wire.begin(SHT41_SDA, SHT41_SCL);
    Wire.setClock(100000);
    delay(10);

    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {
        Serial.println("ERROR: SHT41 communication failed");
        return false;
    }

    delay(10);
    Serial.println("SHT41 communication OK");

    return true;
}

// =====================================================
// Read SHT41
// =====================================================

bool readSHT41(float& temperature, float& humidity) {
    Wire.beginTransmission(SHT41_ADDRESS);
    Wire.write(SHT41_MEASURE_HIGH_PRECISION);

    if (Wire.endTransmission() != 0) {
        Serial.println("ERROR: SHT41 measurement command failed");
        return false;
    }

    delay(10);

    uint8_t received = Wire.requestFrom(
        SHT41_ADDRESS,
        (uint8_t)6);

    if (received != 6) {
        Serial.println("ERROR: SHT41 invalid response length");
        return false;
    }

    uint8_t data[6];

    for (uint8_t i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }

    if (sht41Crc8(data, 2) != data[2]) {
        Serial.println("ERROR: SHT41 temperature CRC failed");
        return false;
    }

    if (sht41Crc8(&data[3], 2) != data[5]) {
        Serial.println("ERROR: SHT41 humidity CRC failed");
        return false;
    }

    uint16_t rawTemperature =
        ((uint16_t)data[0] << 8) | data[1];

    uint16_t rawHumidity =
        ((uint16_t)data[3] << 8) | data[4];

    temperature =
        -45.0f + 175.0f *
                     ((float)rawTemperature / 65535.0f);

    humidity =
        -6.0f + 125.0f *
                    ((float)rawHumidity / 65535.0f);

    if (humidity < 0.0f) {
        humidity = 0.0f;
    }

    if (humidity > 100.0f) {
        humidity = 100.0f;
    }
    return true;
}

// =====================================================
// Setup BME280
// =====================================================

bool setupBME280() {
    if (!bme.begin(BME280_I2C_ADDRESS, &Wire)) {
        Serial.println(
            "ERROR: BME280 communication failed");

        return false;
    }

    Serial.println("BME280 communication OK");

    return true;
}

// ==================================wq===================
// BME280 recovery
// =====================================================

bool recoverBME280() {
    Serial.println("BME280 recovery attempt...");

    if (!bme.begin(BME280_I2C_ADDRESS, &Wire)) {
        Serial.println("BME280 recovery failed");
        return false;
    }

    Serial.println("BME280 recovery OK");
    return true;
}

// =====================================================
// Read BME280
// =====================================================

// =====================================================
// Read BME280
// =====================================================

bool readBME280(
    float& temperature,
    float& humidity,
    float& pressure) {

    temperature = 0.0f;
    humidity = 0.0f;
    pressure = 0.0f;

    float readingTemperature =
        bme.readTemperature();

    float readingHumidity =
        bme.readHumidity();

    float readingPressure =
        bme.readPressure() / 100.0F;

    Serial.printf(
        "BME280 RAW | T=%.2f C | RH=%.2f %% | P=%.2f hPa\n",
        readingTemperature,
        readingHumidity,
        readingPressure);

    bool temperatureValid =
        isfinite(readingTemperature) &&
        readingTemperature > -40.0F &&
        readingTemperature < 85.0F;

    bool humidityValid =
        isfinite(readingHumidity) &&
        readingHumidity >= 0.0F &&
        readingHumidity <= 100.0F;

    bool pressureValid =
        isfinite(readingPressure) &&
        readingPressure >= 300.0F &&
        readingPressure <= 1100.0F;

    // -------------------------------------------------
    // Recovery if any BME280 reading is invalid
    // -------------------------------------------------

    if (!temperatureValid ||
        !humidityValid ||
        !pressureValid) {

        Serial.println(
            "ERROR: BME280 reading invalid");

        if (!recoverBME280()) {
            return false;
        }

        readingTemperature =
            bme.readTemperature();

        readingHumidity =
            bme.readHumidity();

        readingPressure =
            bme.readPressure() / 100.0F;

        temperatureValid =
            isfinite(readingTemperature) &&
            readingTemperature > -40.0F &&
            readingTemperature < 85.0F;

        humidityValid =
            isfinite(readingHumidity) &&
            readingHumidity >= 0.0F &&
            readingHumidity <= 100.0F;

        pressureValid =
            isfinite(readingPressure) &&
            readingPressure >= 300.0F &&
            readingPressure <= 1100.0F;

        if (!temperatureValid ||
            !humidityValid ||
            !pressureValid) {

            Serial.println(
                "ERROR: BME280 readings still invalid after recovery");

            return false;
        }
    }

    temperature = readingTemperature;
    humidity = readingHumidity;
    pressure = readingPressure;

    return true;
}

// =====================================================
// Send SENSOR_DATA
// =====================================================

void sendSensorData(uint16_t requestSequence) {

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    uint8_t sensorFlags = 0;

    // -------------------------------------------------
    // Read BME280
    // -------------------------------------------------

    float bmeTemperature = 0.0f;
    float bmeHumidity = 0.0f;
    float bmePressure = 0.0f;

    bool bme280ReadOK = false;

    if (ENABLE_BME280) {

        bme280ReadOK =
            readBME280(
                bmeTemperature,
                bmeHumidity,
                bmePressure);

        if (bme280ReadOK) {

            // Pressure always comes from BME280
            pressure = bmePressure;

            sensorFlags |=
                FLAG_PRESSURE_VALID;

            // Temperature source selection
            if (USE_BME280_TEMPERATURE) {

                temperature =
                    bmeTemperature;

                sensorFlags |=
                    FLAG_TEMPERATURE_VALID;
            }

            // Humidity source selection
            if (USE_BME280_HUMIDITY) {

                humidity =
                    bmeHumidity;

                sensorFlags |=
                    FLAG_HUMIDITY_VALID;
            }
        } else {

            Serial.println(
                "WARNING: BME280 read failed");
        }
    }

    // -------------------------------------------------
    // SHT41
    // -------------------------------------------------

    if (ENABLE_SHT41) {

        float shtTemperature = 0.0f;
        float shtHumidity = 0.0f;

        if (readSHT41(
                shtTemperature,
                shtHumidity)) {

            // Use SHT41 temperature if selected
            if (!USE_BME280_TEMPERATURE) {

                temperature =
                    shtTemperature;

                sensorFlags |=
                    FLAG_TEMPERATURE_VALID;
            }

            // Use SHT41 humidity if selected
            if (!USE_BME280_HUMIDITY) {

                humidity =
                    shtHumidity;

                sensorFlags |=
                    FLAG_HUMIDITY_VALID;
            }
        } else {

            Serial.println(
                "WARNING: SHT41 read failed");
        }
    }

    // -------------------------------------------------
    // Build SENSOR_DATA packet
    // -------------------------------------------------

    SensorDataPacket packet{};

    packet.header.protocolVersion =
        PROTOCOL_VERSION;

    packet.header.packetType =
        static_cast<uint8_t>(
            PacketType::SENSOR_DATA);

    packet.header.nodeId =
        NODE_ID;

    packet.header.flags =
        sensorFlags;

    packet.header.sequence =
        requestSequence;

    packet.header.payloadLength =
        sizeof(SensorDataPayload);

    packet.header.crc16 = 0;

    packet.header.reserved = 0;

    packet.payload.temperature =
        temperature;

    packet.payload.humidity =
        humidity;

    packet.payload.pressure =
        pressure;

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

/* sht41 fails to read temperature and humidity, bme280 fails to read pressure, or both sensors fail to read
   in any of these cases, the node will not send a SENSOR_DATA packet. The base station will not receive any data from the node.
   This is a design choice to ensure that only valid sensor data is transmitted. If a sensor fails, it is better to not send any data
   than to send invalid data. The base station can then take appropriate action, such as logging the error or alerting the user.

void sendSensorData(uint16_t requestSequence) {
    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    uint8_t sensorFlags = 0;

    // -------------------------------------------------
    // SHT41
    // -------------------------------------------------

    if (ENABLE_SHT41) {
        if (!readSHT41(temperature, humidity)) {
            Serial.println("ERROR: SENSOR_READ_FAILED");
            return;
        }

        sensorFlags |= FLAG_TEMPERATURE_VALID;
        sensorFlags |= FLAG_HUMIDITY_VALID;
    }

    // -------------------------------------------------
    // BME280
    // -------------------------------------------------

    if (ENABLE_BME280) {
        if (readBME280(pressure)) {
            sensorFlags |= FLAG_PRESSURE_VALID;
        } else {
            Serial.println("WARNING: BME280 pressure unavailable");
        }
    }

    // -------------------------------------------------
    // Build SENSOR_DATA packet
    // -------------------------------------------------

    SensorDataPacket packet{};

    packet.header.protocolVersion = PROTOCOL_VERSION;
    packet.header.packetType = static_cast<uint8_t>(PacketType::SENSOR_DATA);
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
            "TX: SENSOR_DATA | Node=%u | Seq=%u | "
            "Flags=0x%02X | T=%.2f C | RH=%.2f %% | P=%.2f hPa\n",
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
} */

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

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.printf("NODE ID: %u\n", NODE_ID);
    Serial.println("================================");

    // Initialize I2C bus
    Wire.begin(SHT41_SDA, SHT41_SCL);
    Wire.setClock(100000);
    delay(10);

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
        if (!setupSHT41()) {
            Serial.println("SHT41 FAILED");
            Serial.println(
                "SHT41 unavailable - continuing without temperature/humidity");
        }
    }

    /*
    if (ENABLE_SHT41) {
        if (!setupSHT41()) {
            Serial.println("SHT41 FAILED");
            Serial.println("SYSTEM HALTED");

            while (true) {
                delay(1000);
            }
        }
    } */

    if (ENABLE_BME280) {
        if (!setupBME280()) {
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