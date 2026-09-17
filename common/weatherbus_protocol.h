#pragma once

#include <Arduino.h>

namespace WeatherBus {

// =====================================================
// Protocol
// =====================================================

constexpr uint8_t PROTOCOL_VERSION = 0x01;

// =====================================================
// Packet Types
// =====================================================

enum class PacketType : uint8_t {
    SENSOR_DATA  = 0x01,
    DATA_REQUEST = 0x02
};

// =====================================================
// Sensor Data Flags
// =====================================================

constexpr uint8_t FLAG_TEMPERATURE_VALID = 0x01;
constexpr uint8_t FLAG_HUMIDITY_VALID    = 0x02;
constexpr uint8_t FLAG_PRESSURE_VALID    = 0x04;

// =====================================================
// WeatherBus Header
// =====================================================

#pragma pack(push, 1)

struct Header {
    uint8_t  protocolVersion;
    uint8_t  packetType;
    uint8_t  nodeId;
    uint8_t  flags;
    uint16_t sequence;
    uint16_t payloadLength;
    uint16_t crc16;
    uint16_t reserved;
};

// =====================================================
// Sensor Data Payload
// =====================================================

struct SensorDataPayload {
    float temperature;
    float humidity;
    float pressure;
};

// =====================================================
// Sensor Data Packet
// =====================================================

struct SensorDataPacket {
    Header header;
    SensorDataPayload payload;
};

#pragma pack(pop)

static_assert(sizeof(Header) == 12,
              "WeatherBus Header must be exactly 12 bytes");

static_assert(sizeof(SensorDataPayload) == 12,
              "SensorDataPayload must be exactly 12 bytes");

static_assert(sizeof(SensorDataPacket) == 24,
              "SensorDataPacket must be exactly 24 bytes");
}