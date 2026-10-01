# WeatherBus

**Development Version:** 1.0
**Status:** In Development
**Last Updated:** 2026-10-01

---

## Overview

**WeatherBus V1.0** is an ESP32-C3 based experimental wireless environmental monitoring system using **ESP-NOW** for communication between a Base Station and sensor Nodes.

The project is currently under active development and hardware testing.

The current development focuses on:

* Wireless sensor communication
* Local sensor monitoring
* Remote sensor Nodes
* RTC timestamping
* SD card data logging
* Sensor data validation
* Node timeout and recovery handling
* Modular firmware development

---

## Development Status

**Current Focus: RTC + SD Integration**

Current development configuration:

```text
1 × WeatherBus Base
1 × WeatherBus Node
ESP32-C3
ESP-NOW
SHT41
BME280
DS3231 RTC
MicroSD
```

The current configuration is intended for **development and hardware testing**.

Only one remote Node is currently enabled. The architecture may be expanded to support additional Nodes as development progresses.

---

## Project Stage

WeatherBus is **not considered a production-ready system**.

Firmware structure, hardware configuration, communication protocol, logging system and sensor management may change during development.

Current development priorities include:

* Hardware stability
* Sensor reliability
* ESP-NOW communication
* RTC timestamp accuracy
* SD logging reliability
* Long-duration testing
* Firmware architecture refinement

---

# System Architecture

```text
                    WeatherBus Base
                       ESP32-C3
                           │
             ┌─────────────┴─────────────┐
             │                           │
       Local Sensor                  ESP-NOW
         Manager                        │
             │                         │
          SHT41                     Node 1
             │                      ESP32-C3
             │                         │
             │                       BME280
             │                         │
             └──────────┬──────────────┘
                        │
                 Sensor Data
                        │
                   RTCManager
                 DS3231 Timestamp
                        │
                    SDLogger
                        │
                  log_wb2.csv
```

The Base currently handles two sensor data sources:

1. **Local sensor**
2. **Remote Node sensor**

Both can be stored on the SD card with RTC timestamps.

---

# Communication

Transport:

```text
ESP-NOW
```

Protocol:

```text
WeatherBus Protocol v0x01
```

Packet types:

```cpp
SENSOR_DATA  = 0x01
DATA_REQUEST = 0x02
```

Communication flow:

```text
Base
 ↓
DATA_REQUEST
 ↓
Node
 ↓
SENSOR_DATA
 ↓
Base
```

The Node responds to valid `DATA_REQUEST` packets from the Base.

---

# Sensor Data Protocol

`SensorDataPayload`:

```cpp
struct SensorDataPayload {
    float temperature;
    float humidity;
    float pressure;
};
```

WeatherBus sensor flags:

```cpp
FLAG_TEMPERATURE_VALID = 0x01;
FLAG_HUMIDITY_VALID    = 0x02;
FLAG_PRESSURE_VALID    = 0x04;
```

Packet sizes:

```text
Header              12 bytes
SensorDataPayload   12 bytes
Total               24 bytes
```

---

# Base Station

## LocalSensorManager

The Base contains `LocalSensorManager` for local environmental sensing.

Current configuration:

```cpp
ENABLE_SHT41  = true;
ENABLE_BME280 = false;
```

I²C:

```text
SDA   = GPIO 8
SCL   = GPIO 9
Clock = 100 kHz
```

### SHT41

```text
Address: 0x44
```

Current implementation includes:

* I²C communication
* High precision measurement
* Temperature conversion
* Humidity conversion
* CRC-8 validation
* Humidity range limiting

---

# Remote Sensor Node

Current Node:

```cpp
NODE_ID = 1;
```

Current sensor configuration:

```cpp
ENABLE_SHT41  = false;
ENABLE_BME280 = true;
```

BME280:

```text
I²C Address: 0x76
SDA: GPIO 8
SCL: GPIO 9
```

Current Node provides:

```text
Temperature
Humidity
Pressure
```

Sensor data is validated before transmission.

---

# Polling Engine

The Base uses `PollingEngine` to manage communication with remote Nodes.

State machine:

```text
IDLE
  ↓
SEND_REQUEST
  ↓
WAIT_RESPONSE
  ↓
IDLE
```

Current timing:

```text
Polling Interval : 2000 ms
Response Timeout : 200 ms
```

The engine handles:

* Node polling
* Sequence numbering
* Response validation
* Node ID validation
* Sensor data validation
* Timeout handling
* Online/offline status

---

# Node Manager

`NodeManager` maintains configured Node information.

Current development configuration:

```cpp
MAX_NODES = 1;
```

Each Node contains:

```text
Node ID
MAC Address
Online Status
Last Response
```

The one-Node configuration is intentional for the current hardware testing stage.

---

# RTC Integration

The Base uses a **DS3231 RTC** through `RTCManager`.

Modules:

```text
src/base/RTCManager.cpp
src/base/RTCManager.h
```

I²C:

```text
SDA = GPIO 8
SCL = GPIO 9
```

Available functions:

```cpp
begin()
update()
isOK()
now()
setDateTime()
```

RTC timestamps are used by the SD logging system.

Timestamp format:

```text
YYYY-MM-DD HH:MM:SS
```

Example:

```text
2026-10-01 10:30:15
```

The firmware currently does not automatically overwrite the RTC using compile time.

---

# SD Card Integration

The Base uses `SDLogger` for sensor data logging.

Modules:

```text
src/base/SDLogger.cpp
src/base/SDLogger.h
```

SD interface:

```text
SCK  = GPIO 4
MISO = GPIO 5
MOSI = GPIO 6
CS   = GPIO 10
```

SPI speed:

```text
10 MHz
```

Log file:

```text
log_wb2.csv
```

CSV header:

```text
Timestamp,
Node_ID,
Temperature_C,
Humidity_PCT,
Pressure_hPa,
Flags
```

Example:

```text
2026-10-01 10:31:02,1,32.81,62.45,1012.32,0x07
2026-10-01 10:31:04,0,32.95,68.82,NA,0x03
```

`Node_ID = 0` represents the Base local sensor.

Unavailable sensor values are stored as:

```text
NA
```

---

# Data Flow

### Remote Node

```text
Node Sensor
    ↓
SENSOR_DATA
    ↓
ESP-NOW
    ↓
Base RX Callback
    ↓
PollingEngine Validation
    ↓
SDLogger
```

### Local Sensor

```text
LocalSensorManager
    ↓
SensorDataPayload
    ↓
RTC Timestamp
    ↓
SDLogger
    ↓
log_wb2.csv
```

SD writing is performed from the main loop rather than directly inside the ESP-NOW callback.

---

# Project Structure

```text
WeatherBus_2/
│
├── common/
│   └── weatherbus_protocol.h
│
├── include/
│
├── lib/
│
├── src/
│   ├── base/
│   │   ├── main.cpp
│   │   ├── local_sensor_manager.cpp
│   │   ├── local_sensor_manager.h
│   │   ├── node_manager.cpp
│   │   ├── node_manager.h
│   │   ├── polling_engine.cpp
│   │   ├── polling_engine.h
│   │   ├── RTCManager.cpp
│   │   ├── RTCManager.h
│   │   ├── SDLogger.cpp
│   │   └── SDLogger.h
│   │
│   └── node/
│       └── main.cpp
│
├── test/
│
├── Documentation/
│   └── Upload and Serial
│
└── platformio.ini
```

---

# PlatformIO

Platform:

```text
Espressif32
```

Board:

```text
LOLIN C3 Mini
```

Framework:

```text
Arduino
```

Serial:

```text
115200 baud
```

Available environments:

```text
base
node
```

Build Base:

```bash
pio run -e base
```

Upload Base:

```bash
pio run -e base -t upload
```

Build Node:

```bash
pio run -e node
```

Upload Node:

```bash
pio run -e node -t upload
```

---

# Dependencies

### Base

```text
Adafruit BME280 Library
Adafruit Unified Sensor
SdFat
RTClib
```

### Node

```text
Adafruit BME280 Library
SdFat
```

---

# Development Phases

```text
Phase 1
PASS

Phase 2A
PASS

Phase 2B.1
PASS

Phase 2B.2
PASS

Phase 2B.3
PASS

Phase 2B.4
PASS

Phase 2B.5
PASS

Phase 2B.6
PASS

Phase 2B.7
PASS

Phase 2C.1
PASS

Phase 2C.2
PASS

Phase 2C.3
PASS

Phase 2C.4
PASS

Phase 2C.5
PASS

Phase 2C.6
CONDITIONAL PASS

RTC Integration
IMPLEMENTED

SD Integration
IMPLEMENTED
```

---

# Current Development Focus

Current development is focused on validating the complete Base data pipeline:

```text
Local Sensor
      +
Remote Node
      ↓
WeatherBus Validation
      ↓
RTC Timestamp
      ↓
SDLogger
      ↓
CSV Storage
```

Current testing priorities:

* Node communication stability
* Local sensor reliability
* RTC timestamp accuracy
* SD write reliability
* Combined local and remote logging
* Node timeout and recovery
* Long-duration operation

---

# Development Notes

WeatherBus is currently an **experimental embedded development project**.

Hardware configuration, firmware architecture, protocol implementation and module interfaces may change during development.

The current one-Node configuration is intended for hardware testing and does not represent the final system configuration.

---

## License

This project is currently maintained as a personal development project.
