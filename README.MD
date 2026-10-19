# WeatherBus

**Development Version:\*\*** 1.0

**Status:\*\*** In Development

**Last Updated:\*\*** 2026-10-14

------------------------------------------------------------------------

## Overview

WeatherBus V1.0 is an experimental ESP32-C3 wireless environmental
monitoring system using ESP-NOW communication.

The system supports local and remote environmental sensors, RTC
timestamping, sensor data validation, fault handling, and MicroSD CSV
logging.

**Current Configuration:\*\***

-   1 × Base Station (ESP32-C3)

-   1 × Remote Node (ESP32-C3)

-   ESP-NOW communication

-   SHT41 (Base)

-   BME280 (Node 1)

-   DS3231 RTC

-   MicroSD storage

## Development Status

| Component \| Status \|

\|---\|---\|

| ESP-NOW Communication \| PASS \|

| WeatherBus Protocol V1.0 \| PASS \|

| Local Sensor (SHT41) \| PASS \|

| Remote Sensor (BME280) \| PASS \|

| Polling Engine \| PASS \|

| Node Manager \| PASS \|

| Sensor Data Validation \| PASS \|

| BME280 Fault Handling & Recovery \| PASS \|

| Partial / Complete Sensor Failure Tests \| PASS \|

| RTC Integration \| COMPLETED \|

| SD Card Integration \| COMPLETED \|

| SD CSV Validity Verification \| PASS \|
| SD Write / Sync Error Detection \| PASS \|
| SD Startup Recovery (Physical) \| PASS \|
| SD Runtime Recovery (Simulation) \| PASS \|
| SD Runtime Recovery (Physical) \| PASS \|
| SD Long-Run CSV Structural Integrity \| PASS (observed data gaps) \|

| ESP-NOW Response Handoff \| PASS \|

| Late / Duplicate Response Tests \| PASS \|

| Local Sensor Logging Validation \| PASS \|

| Single Base + Node Hardware Test \| PASS \|

| Multi-Node Hardware Test \| PENDING \|

| Long-Duration System Stability Test \| PENDING \|

## System Architecture

``` text

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

Modular firmware architecture with separated application logic,
services, interfaces, and hardware drivers.

``` text

Application

   |

Services

   |

Interfaces

   ^

   |

Drivers
```

## Communication

**Transport:\*\*** ESP-NOW

**Protocol Version:\*\*** `0x01`

**Packet Types:\*\***

``` cpp

SENSOR_DATA  = 0x01

DATA_REQUEST = 0x02
```

**Packet Structure:\*\***

| Field \| Size \|

\|---\|---\|

| WeatherBus Header \| 12 bytes \|

| SensorDataPayload \| 12 bytes \|

| Total SENSOR_DATA Packet \| 24 bytes \|

``` cpp

struct SensorDataPayload {

   float temperature;

   float humidity;

   float pressure;

};
```

**Validation Flags:\*\***

``` cpp

FLAG_TEMPERATURE_VALID = 0x01

FLAG_HUMIDITY_VALID    = 0x02

FLAG_PRESSURE_VALID    = 0x04
```

## Polling Engine

``` text

MAX_NODES           = 1

POLL_INTERVAL_MS    = 2000

RESPONSE_TIMEOUT_MS = 200
```

**State Machine:\*\***

``` text

IDLE

|

SEND_REQUEST

|

WAIT_RESPONSE

|

IDLE
```

Implemented:

-   Node ID and sequence validation

-   Response timeout handling

-   Node online/offline tracking

-   ESP-NOW response handoff synchronization

-   Late and duplicate response handling

## Sensor Configuration

| Parameter \| Base \| Node 1 \|

\|---\|---\|---\|

| MCU \| ESP32-C3 \| ESP32-C3 \|

| Sensor \| SHT41 \| BME280 \|

| Temperature \| Yes \| Yes \|

| Humidity \| Yes \| Yes \|

| Pressure \| No \| Yes \|

| I²C SDA \| GPIO 8 \| GPIO 8 \|

| I²C SCL \| GPIO 9 \| GPIO 9 \|

**SHT41 Address:\*\*** `0x44`

**BME280 Address:\*\*** `0x76`

## Sensor Validity & Fault Handling

BME280 supports independent parameter validation, sensor recovery
attempts, and continued ESP-NOW communication during sensor failures.

**Hardware Test Results --- 2026-10-14\*\***

| Test \| Scenario \| Flags \| Status \|

\|---\|---\|---\|---\|

| T0 \| Normal Readings \| `0x07` \| PASS \|

| T1 \| Temperature Invalid \| `0x06` \| PASS \|

| T2 \| Humidity Invalid \| `0x05` \| PASS \|

| T3 \| Pressure Invalid \| `0x03` \| PASS \|

| T4 \| All Parameters Invalid \| `0x00` \| PASS \|

| Recovery \| Return to Normal Operation \| `0x07` \| PASS \|

**Verified Behaviour:\*\***

-   Independent temperature, humidity, and pressure validity flags

-   Invalid readings excluded from valid sensor values

-   BME280 recovery attempts on invalid readings

-   Continued `SENSOR_DATA` transmission during complete sensor failure

-   Base receives `0x00` flags without treating the packet as valid
    measurements

-   Polling continues during sensor failure

-   Normal sensor operation restored after fault simulation

**Observed Response Time:\*\***

-   Normal operation: approximately 6 ms

-   Injected sensor failure with recovery attempt: approximately
    131--149 ms

-   Response timeout: 200 ms

## RTC & SD Logging

**RTC:\*\*** DS3231

**Storage:\*\*** MicroSD (SdFat)

**Log File:\*\*** `log_wb2.csv`

**SD SPI Configuration:\*\***

``` text

SCK   = GPIO 4

MISO  = GPIO 5

MOSI  = GPIO 6

CS    = GPIO 10

Speed = 10 MHz
```

**CSV Format:\*\***

``` csv

Timestamp,Node_ID,Temperature_C,Humidity_PCT,Pressure_hPa,Flags

2026-10-14 22:36:24,1,NA,54.82,1014.15,0x06

2026-10-14 22:40:23,1,24.55,NA,1014.21,0x05

2026-10-14 22:07:36,1,NA,NA,NA,0x00
```

-   `Node_ID = 0`: Base local sensor

-   `Node_ID = 1`: Remote Node

-   Invalid or unavailable parameters: `NA`

-   Timestamp: `YYYY-MM-DD HH:MM:SS`

-   SD writes are handled outside ESP-NOW receive callbacks

-   Invalid local sensor readings are excluded from logging

-   Remote sensor validity flags are preserved in CSV records

-   SD initialization, file-open, write, and sync failures are checked

-   Logging is paused when SD operations fail; recovery is retried at
    5-second intervals

-   CSV file name is centralized as `LOG_FILE` (`log_wb2.csv`)

-   SD writes and sync results are checked before a record is reported
    as successfully logged

**CSV Verification --- 2026-10-14\*\***

| Test \| Node Records \| Status \|

\|---\|---:\|---\|

| T0 --- Normal \| 575 \| PASS \|

| T1 --- Temperature Invalid \| 109 \| PASS \|

| T2 --- Humidity Invalid \| 69 \| PASS \|

| T3 --- Pressure Invalid \| 135 \| PASS \|

| T4 --- All Invalid \| 583 \| PASS \|

**Total Verified Records:\*\*** 2,997

**Base Records:\*\*** 1,526

**Node 1 Records:\*\*** 1,471

## PlatformIO

**Platform:\*\*** Espressif32

**Board:\*\*** LOLIN C3 Mini

**Framework:\*\*** Arduino

**Serial Monitor:\*\*** 115200 baud

**Build:\*\***

``` bash

pio run -e base

pio run -e node
```

**Upload:\*\***

``` bash

pio run -e base -t upload

pio run -e node -t upload
```

## Development Milestones

| Phase / Component \| Description \| Status \|

\|---\|---\|---\|

| Phase 1 \| ESP-NOW Base ↔ Node Communication \| PASS \|

| Phase 2A \| SHT41 Sensor Integration \| PASS \|

| Phase 2B.1 -- 2B.7 \| Packet Validation, Polling, Timeout & Recovery
  \| PASS \|

| Phase 2C.1 -- 2C.5 \| BME280 Integration, Sensor Flags & Error
  Handling \| PASS \|

| Phase 2C.6 \| Sensor Validation & Regression Testing \| CONDITIONAL
  PASS \|

| RTC Integration \| DS3231, RTCManager & Timestamp \| COMPLETED \|

| SD Integration \| SdFat, CSV Logging, Write/Sync Validation & Recovery
  \| COMPLETED \|

| Transport Abstraction \| ESP-NOW Transport Interface & Driver \|
  COMPLETED \|

| Firmware Refactoring \| Service, Interface & Driver Separation \|
  COMPLETED \|

| Node Sensor Abstraction \| SHT41 / BME280 Driver Separation \|
  COMPLETED \|

| Local Sensor Logging Fix \| Skip Invalid Local Sensor Readings \|
  COMPLETED \|

| ESP-NOW Response Handoff \| Response Synchronization & Validation \|
  COMPLETED \|

| Response Testing \| Normal, Late & Duplicate Responses \| PASS \|

| BME280 Validity Testing \| T0--T4 Sensor Failure Simulation \| PASS \|

| BME280 Fault Handling \| Partial / Complete Failure & Recovery
  Attempts \| PASS \|

| SD CSV Verification \| Validity Flags & NA Storage \| PASS \|

| Sensor Recovery Verification \| Return to Normal Readings \| PASS \|

| Hardware Testing \| Base + 1 Remote Node \| PASS \|

| Multi-Node Testing \| Additional Node Hardware Testing \| PENDING \|

| Long-Duration System Testing \| Continuous Operation & Stability \|
  PENDING \|

**Current Stage:\*\*** Stability Testing & Data Collection

**Current Stage:\*\*** Stability Testing & Data Collection

**Development Baseline:\*\*** `6d2152c`

## Latest Development Updates

**14 October 2026\*\***

-   Improved BME280 sensor validity and fault handling.

-   Verified independent temperature, humidity, and pressure validity
    flags.

-   Completed T0--T4 sensor failure simulation tests.

-   Verified continued ESP-NOW communication during complete sensor
    failure.

-   Confirmed return to normal sensor operation.

-   Verified CSV storage of invalid sensor parameters using `NA`.

-   Confirmed RTC and SD logging operation during testing.

-   Synchronized ESP-NOW response handoff.

-   Verified normal, late, and duplicate responses.

-   Created and pushed `Sensor_Validity_Handling` branch.

**Latest Commit:\*\***

``` text

ce455e3 fix: improve BME280 sensor validity and fault handling
```

## Next Development

-   Merge validated sensor handling changes into `main`

-   Continuous Base + Node data collection

-   Long-duration stability testing

-   Monitor sensor and SD logging reliability

-   Additional multi-node hardware testing

-   Further firmware refinement as required

## Development Notes

WeatherBus V1.0 remains under active development and is not
production-ready.

The current hardware test configuration consists of one Base Station and
one remote Node. Firmware architecture and hardware configuration may
change during development.

## License

Personal development project. License not yet specified.
