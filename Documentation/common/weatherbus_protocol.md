# WeatherBus Communication Protocol

## 1. Purpose

Defines the shared packet structures, packet types and sensor validity
flags used for communication between the Base Station and Sensor Nodes
through ESP-NOW.

Source file: `common/weatherbus_protocol.h`

## 2. Protocol Version

| Property | Value |
|---|---|
| Protocol version | `0x01` |
| Packet header size | 12 bytes |
| Sensor data payload size | 12 bytes |
| Sensor data packet size | 24 bytes |
| Structure packing | 1-byte alignment |

The protocol version identifies the packet format used by communicating
devices.

Both Base Station and Sensor Node must use compatible protocol definitions.

## 3. Packet Types

| Packet Type | Value | Purpose |
|---|---:|---|
| `SENSOR_DATA` | `0x01` | Carries sensor data |
| `DATA_REQUEST` | `0x02` | Requests sensor data from a Node |

## 4. Header Structure

The `Header` structure is exactly 12 bytes.

| Field | Type | Size | Description |
|---|---|---:|---|
| `protocolVersion` | `uint8_t` | 1 byte | Protocol version |
| `packetType` | `uint8_t` | 1 byte | Packet type |
| `nodeId` | `uint8_t` | 1 byte | Identifies the Sensor Node |
| `flags` | `uint8_t` | 1 byte | Packet flags; sensor validity flags are defined |
| `sequence` | `uint16_t` | 2 bytes | Sequence number |
| `payloadLength` | `uint16_t` | 2 bytes | Payload length in bytes |
| `crc16` | `uint16_t` | 2 bytes | Field reserved for CRC-16 |
| `reserved` | `uint16_t` | 2 bytes | Reserved field |

## 5. Sensor Data Payload

The `SensorDataPayload` structure is exactly 12 bytes.

| Field | Type | Size | Description |
|---|---|---:|---|
| `temperature` | `float` | 4 bytes | Temperature value |
| `humidity` | `float` | 4 bytes | Relative humidity value |
| `pressure` | `float` | 4 bytes | Atmospheric pressure value |

The payload contains three floating-point values.

The header's `flags` field indicates which sensor values are valid.

## 6. Sensor Validity Flags

| Flag | Value | Meaning |
|---|---:|---|
| `FLAG_TEMPERATURE_VALID` | `0x01` | Temperature is valid |
| `FLAG_HUMIDITY_VALID` | `0x02` | Humidity is valid |
| `FLAG_PRESSURE_VALID` | `0x04` | Pressure is valid |

Flags can be combined using bitwise OR.

Example:

`0x07` means all three validity flags are set.

A receiver should use these flags to determine which sensor values
are valid rather than assuming that every payload value is valid.

## 7. Packet Layout

### SensorDataPacket

```text
+--------------------------------+
| Header             12 bytes    |
+--------------------------------+
| SensorDataPayload  12 bytes    |
+--------------------------------+
| Total              24 bytes    |
+--------------------------------+