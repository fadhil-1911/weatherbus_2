// =====================================================
// File: src/services/NodeCommunicationService.h
// =====================================================

#include "NodeCommunicationService.h"

#include <cstring>

#include "../node/config/node_settings.h"
#include "../common/weatherbus_protocol.h"

using namespace WeatherBus;

NodeCommunicationService* NodeCommunicationService::instance = nullptr;

// Protect shared request data between callback and loop.
static portMUX_TYPE requestMux = portMUX_INITIALIZER_UNLOCKED;

// =====================================================
// Constructor
// =====================================================

NodeCommunicationService::NodeCommunicationService(
    ITransport& transport,
    ISensorSource& bme280Source,
    ISensorSource& sht41Source)
    : transport(transport),
      bme280Source(bme280Source),
      sht41Source(sht41Source) {
}

// =====================================================
// Begin communication
// =====================================================

bool NodeCommunicationService::begin(
    const TransportAddress& baseAddress) {

    this->baseAddress = baseAddress;

    instance = this;

    if (!transport.begin(onDataReceived)) {
        Serial.println(
            "ERROR: ESP-NOW initialization failed");

        instance = nullptr;
        return false;
    }

    if (!transport.addPeer(this->baseAddress)) {
        Serial.println(
            "ERROR: Failed to add Base peer");

        ready = false;
        return false;
    }

    ready = true;
    return true;
}

// =====================================================
// Receive callback
// =====================================================

void NodeCommunicationService::onDataReceived(
    const uint8_t* data,
    size_t length) {

    if (instance == nullptr) {
        return;
    }

    instance->handleReceivedData(data, length);
}

// =====================================================
// Validate DATA_REQUEST
// =====================================================

void NodeCommunicationService::handleReceivedData(
    const uint8_t* data,
    size_t length) {

    if (data == nullptr || length != sizeof(Header)) {
        Serial.printf(
            "RX ERROR: Invalid packet size | "
            "Received=%u | Expected=%u\n",
            static_cast<unsigned>(length),
            static_cast<unsigned>(sizeof(Header)));
        return;
    }

    Header request{};
    memcpy(&request, data, sizeof(request));

    if (request.protocolVersion != PROTOCOL_VERSION) {
        Serial.printf(
            "RX ERROR: Invalid protocol version | "
            "Received=%u | Expected=%u\n",
            request.protocolVersion,
            PROTOCOL_VERSION);
        return;
    }

    if (request.packetType !=
        static_cast<uint8_t>(PacketType::DATA_REQUEST)) {

        Serial.printf(
            "RX ERROR: Invalid packet type | Received=0x%02X\n",
            request.packetType);
        return;
    }

    if (request.nodeId != NODE_ID) {
        Serial.printf(
            "RX ERROR: Invalid Node ID | "
            "Received=%u | This Node=%u\n",
            request.nodeId,
            NODE_ID);
        return;
    }

    if (request.payloadLength != 0) {
        Serial.printf(
            "RX ERROR: Invalid payload length | "
            "Received=%u | Expected=0\n",
            request.payloadLength);
        return;
    }

    bool accepted = false;

    portENTER_CRITICAL(&requestMux);

    if (!requestPending) {
        pendingSequence = request.sequence;
        requestPending = true;
        accepted = true;
    }

    portEXIT_CRITICAL(&requestMux);

    if (accepted) {
        Serial.printf(
            "RX: DATA_REQUEST | Node=%u | Seq=%u\n",
            NODE_ID,
            request.sequence);
    } else {
        Serial.printf(
            "RX WARNING: Request pending | Seq=%u ignored\n",
            request.sequence);
    }
}

// =====================================================
// Process pending request in loop()
// =====================================================

void NodeCommunicationService::update() {
    if (!ready) {
        return;
    }

    uint16_t sequence = 0;
    bool hasRequest = false;

    portENTER_CRITICAL(&requestMux);

    if (requestPending) {
        sequence = pendingSequence;
        requestPending = false;
        hasRequest = true;
    }

    portEXIT_CRITICAL(&requestMux);

    if (hasRequest) {
        processRequest(sequence);
    }
}

// =====================================================
// Read sensors and send SENSOR_DATA
// =====================================================

void NodeCommunicationService::processRequest(
    uint16_t requestSequence) {

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    uint8_t sensorFlags = 0;

    // -------------------------------------------------
    // Read BME280
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
            Serial.println(
                "WARNING: BME280 read failed");
        }
    }

    // -------------------------------------------------
    // Read SHT41
    // -------------------------------------------------

    if (ENABLE_SHT41) {
        SensorReading shtReading;

        if (sht41Source.read(shtReading)) {

            if (!USE_BME280_TEMPERATURE &&
                shtReading.temperatureValid) {

                temperature = shtReading.temperature;
                sensorFlags |= FLAG_TEMPERATURE_VALID;
            }

            if (!USE_BME280_HUMIDITY &&
                shtReading.humidityValid) {

                humidity = shtReading.humidity;
                sensorFlags |= FLAG_HUMIDITY_VALID;
            }

        } else {
            Serial.println(
                "WARNING: SHT41 read failed");
        }
    }

    // TEST ONLY: Simulate late response
    //delay(300);

    // -------------------------------------------------
    // Build SENSOR_DATA packet
    // -------------------------------------------------

    SensorDataPacket packet{};

    packet.header.protocolVersion = PROTOCOL_VERSION;
    packet.header.packetType =
        static_cast<uint8_t>(PacketType::SENSOR_DATA);

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
    // Send through ITransport
    // -------------------------------------------------

    bool sent = transport.send(
        baseAddress,
        reinterpret_cast<const uint8_t*>(&packet),
        sizeof(packet));

    if (sent) {
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

        // -------------------------------------------------
        // TEST ONLY: Duplicate SENSOR_DATA
        // -------------------------------------------------
        /*
        bool duplicateSent = transport.send(
            baseAddress,
            reinterpret_cast<const uint8_t*>(&packet),
            sizeof(packet));

        Serial.printf(
            "TEST: Duplicate SENSOR_DATA | Seq=%u | Send=%s\n",
            packet.header.sequence,
            duplicateSent ? "OK" : "FAILED"); */
    } else {
        Serial.println(
            "TX ERROR: Transport send failed");
    }
}
