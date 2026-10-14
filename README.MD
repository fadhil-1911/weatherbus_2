# WeatherBus

**Development Version:** 1.0  
**Status:** In Development  
**Last Updated:** 2026-10-14

---

## Overview

WeatherBus V1.0 is an experimental wireless environmental monitoring system built on ESP32-C3 using ESP-NOW communication.

The system supports local and remote environmental sensors, RTC timestamping, and MicroSD data logging.

**Current Configuration:**
- 1 × Base Station (ESP32-C3)
- 1 × Remote Node (ESP32-C3)
- ESP-NOW communication
- SHT41 (Base)
- BME280 (Node)
- DS3231 RTC
- MicroSD storage

The current configuration is intended for development, hardware testing, and continuous environmental data collection.

## Development Status

| Component | Status |
|---|---|
| ESP-NOW Communication | PASS |
| WeatherBus Protocol V1.0 | PASS |
| Local Sensor (SHT41) | PASS |
| Remote Sensor (BME280) | PASS |
| Polling Engine | PASS |
| Node Manager | PASS |
| Sensor Data Validation | PASS |
| RTC Integration | IMPLEMENTED |
| SD Card Integration | IMPLEMENTED |
| ESP-NOW Response Handoff | PASS |
| Late/Duplicate Response Tests | PASS |
| Local Sensor Logging Validation | PASS |
| Single Base + Node Hardware Test | PASS |
| Multi-Node Hardware Test | PENDING |
| Long-Duration Stability Test | PENDING |

## System Architecture

```text
                WeatherBus Base
                   ESP32-C3
                       |
           +-----------+-----------+
           |                       |
      Local Sensor               ESP-NOW
         SHT41                     |
           |                    Node 1
           |                   ESP32-C3
           |                       |
           |                     BME280
           |                       |
           +-----------+-----------+
                       |
                Data Validation
                       |
                 RTC Timestamp
                       |
                   SDLogger
                       |
                  log_wb2.csv
```

## Firmware Architecture

WeatherBus uses a modular firmware architecture separating application logic, services, interfaces, and hardware drivers.

```text
Application
    |
  Services
    |
 Interfaces
    ^
    |
  Drivers
```

The architecture is intended to simplify hardware changes, testing, and future expansion.

## Communication

**Transport:** ESP-NOW  
**Protocol Version:** `0x01`

**Packet Types:**

```cpp
SENSOR_DATA  = 0x01
DATA_REQUEST = 0x02
```

**Packet Structure:**

| Field | Size |
|---|---|
| WeatherBus Header | 12 bytes |
| SensorDataPayload | 12 bytes |
| Total | 24 bytes |

**Sensor Data:**

```cpp
struct SensorDataPayload {
    float temperature;
    float humidity;
    float pressure;
};
```

**Validation Flags:**

```cpp
FLAG_TEMPERATURE_VALID = 0x01
FLAG_HUMIDITY_VALID    = 0x02
FLAG_PRESSURE_VALID    = 0x04
```

## Polling Engine

**Current Configuration:**

```text
MAX_NODES         = 1
POLL_INTERVAL_MS  = 2000
RESPONSE_TIMEOUT  = 200
```

**State Machine:**

```text
IDLE
  |
SEND_REQUEST
  |
WAIT_RESPONSE
  |
IDLE
```

Implemented functionality:
- Node ID and sequence validation
- Sensor data validation
- Response timeout handling
- Online/offline status tracking
- ESP-NOW response handoff synchronization
- Late and duplicate response handling

## Sensor Configuration

| Parameter | Base | Node 1 |
|---|---|---|
| MCU | ESP32-C3 | ESP32-C3 |
| Sensor | SHT41 | BME280 |
| Temperature | Yes | Yes |
| Humidity | Yes | Yes |
| Pressure | No | Yes |
| I²C SDA | GPIO 8 | GPIO 8 |
| I²C SCL | GPIO 9 | GPIO 9 |

**SHT41 Address:** `0x44`  
**BME280 Address:** `0x76`

## RTC & SD Logging

**RTC:** DS3231  
**Storage:** MicroSD (SdFat)  
**Log File:** `log_wb2.csv`

**SD SPI Configuration:**

```text
SCK  = GPIO 4
MISO = GPIO 5
MOSI = GPIO 6
CS   = GPIO 10
Speed = 10 MHz
```

**CSV Format:**

```csv
Timestamp,Node_ID,Temperature_C,Humidity_PCT,Pressure_hPa,Flags
2026-10-01 10:31:02,1,32.81,62.45,1012.32,0x07
2026-10-01 10:31:04,0,32.95,68.82,NA,0x03
```

- `Node_ID = 0`: Base local sensor
- `Node_ID = 1`: Remote Node
- Unavailable values: `NA`
- Timestamp: `YYYY-MM-DD HH:MM:SS`
- SD writing is handled outside the ESP-NOW receive callback.
- Invalid local sensor readings are excluded from logging.

## PlatformIO

**Platform:** Espressif32  
**Board:** LOLIN C3 Mini  
**Framework:** Arduino  
**Serial Monitor:** 115200 baud

**Build:**

```bash
pio run -e base
pio run -e node
```

**Upload:**

```bash
pio run -e base -t upload
pio run -e node -t upload
```

## Development Milestones

| Phase / Component | Description | Status |
|---|---|---|
| Phase 1 | ESP-NOW Base ↔ Node Communication | PASS |
| Phase 2A | SHT41 Sensor Integration | PASS |
| Phase 2B.1 – 2B.7 | Packet Validation, Polling, Timeout & Recovery | PASS |
| Phase 2C.1 – 2C.5 | BME280 Integration, Sensor Flags & Error Handling | PASS |
| Phase 2C.6 | Sensor Validation & Regression Testing | CONDITIONAL PASS |
| RTC Integration | DS3231, RTCManager & Timestamp | COMPLETED |
| SD Integration | SdFat, CSV Logging & Sensor Data Storage | COMPLETED |
| Transport Abstraction | ESP-NOW Transport Interface & Driver | COMPLETED |
| Firmware Refactoring | Service, Interface & Driver Separation | COMPLETED |
| Node Sensor Abstraction | SHT41 / BME280 Driver Separation | COMPLETED |
| Local Sensor Logging Fix | Skip Invalid Local Sensor Readings | COMPLETED |
| ESP-NOW Response Handoff | Response Synchronization & Validation | COMPLETED |
| Response Testing | Normal, Late & Duplicate Responses | PASS |
| Hardware Testing | Base + 1 Remote Node | PASS |
| Multi-Node Testing | Additional Node Hardware Testing | PENDING |
| Long-Duration Testing | Continuous Operation & Stability | PENDING |

**Current Stage:** Stability Testing & Data Collection

**Development Baseline:** `2832261`

## Latest Development Updates

**14 October 2026**

- Fixed local sensor SD logging validation.
- Removed unintended project files.
- Synchronized ESP-NOW response handoff.
- Verified normal, late, and duplicate responses.
- Passed build and hardware tests.
- Updated and pushed changes to `main`.

**Development Baseline:** `2832261`

## Next Development

- Continuous Base + Node data collection
- Long-duration stability testing
- Monitor sensor and SD logging reliability
- Additional multi-node testing when hardware is available
- Further firmware refinement as required

## Development Notes

WeatherBus V1.0 remains under active development and is not production-ready.

The current stable development configuration uses one Base and one remote Node. Hardware configuration, firmware interfaces, and protocol implementation may change.

## License

Personal development project. License not yet specified.