# Base Station — Main Application

## 1. Purpose

Entry point for the WeatherBus Base Station.

Coordinates local sensor acquisition, ESP-NOW communication, Node polling, RTC timestamp management, SD data logging, wireless serial debugging and the web dashboard.

**Source:** `src/base_main/main.cpp`

## 2. Responsibilities

- Initialize Wi-Fi debugging and the web dashboard.
- Initialize I2C and system modules.
- Initialize the local SHT41 sensor service.
- Initialize the SD logger and RTC clock.
- Initialize ESP-NOW transport and register configured Node peers.
- Receive and validate sensor data packets.
- Coordinate remote Node polling through `PollingEngine`.
- Update the web dashboard with local and remote sensor readings.
- Log local and remote sensor readings using RTC timestamps.
- Detect RTC update failures and report recovery events.
- Service wireless debugging and dashboard operations in the main loop.
- Report module failures and recovery events through Serial.

## 3. Module Dependencies

| Module | Responsibility |
|---|---|
| `NodeManager` | Provides configured Node information |
| `ESPNowTransport` | Handles ESP-NOW transport |
| `PollingEngine` | Requests and processes remote Node responses |
| `LocalSensorService` | Reads local sensor measurements |
| `SHT41Source` | Provides local temperature and humidity |
| `RTCClock` | Provides RTC time and status |
| `SDLogger` | Writes sensor data to SD storage |
| `IDataLogger` | Abstract logging interface |
| `WirelessDebugService` | Manages the wireless debug access point and TCP client |
| `DebugSerial` | Provides buffered wireless debug output alongside USB Serial |
| `WebDashboardService` | Manages the web dashboard and sensor data updates |
| `weatherbus_protocol.h` | Defines packet structures and protocol constants |
| `node_config.h` | Provides Base Station Node configuration |
| `Wire` | Initializes the I2C bus |

## 4. Initialization Sequence

The `setup()` function performs these operations:

1. Start Serial communication at 115200 baud.
2. Initialize `WirelessDebugService`.
3. Initialize `WebDashboardService`.
4. Initialize I2C using SDA GPIO 8 and SCL GPIO 9 at 100 kHz.
5. Print the WeatherBus startup banner.
6. Initialize `NodeManager`.
7. Initialize `LocalSensorService`.
8. Initialize the data logger.
9. Initialize `RTCClock` and perform an initial RTC update.
10. Initialize ESP-NOW transport and register configured Node peers.
11. Halt execution if transport initialization or peer registration fails.
12. Initialize `PollingEngine`.
13. Register `onValidNodeResponse()` as the valid response callback.
14. Wait five seconds before returning from `setup()`.

### Initialization Failure Behaviour

| Component | Current Behaviour |
|---|---|
| Wireless debug service | Prints an error and continues |
| Web dashboard | Prints an error and continues |
| Local sensor service | Prints an error and continues |
| SD logger | Prints a warning and continues |
| RTC | Reports successful initialization and update, or update failure after successful initialization |
| ESP-NOW transport | Halts the system if initialization fails |
| Node peer registration | Halts the system if peer registration fails |
| Polling engine | Initialized after successful transport setup |

The return values from wireless debugging, dashboard and local sensor initialization do not halt startup. SD logger failure also does not halt startup.

## 5. Main Loop

The `loop()` function periodically services the following components:

| Operation | Frequency / Trigger |
|---|---|
| `webDashboard.update()` | Every loop iteration |
| `wirelessDebug.update()` | Every loop iteration |
| `transport.update()` | Every loop iteration |
| `pollingEngine.update()` | Every loop iteration |
| `sdLogger.update()` | Every loop iteration |
| `rtcClock.update()` | Approximately every 1000 ms |
| Remote Node logging | When `nodeLogPending` is set |
| Local sensor read | Approximately every 2000 ms |
| Dashboard local data update | After each local sensor read |
| Dashboard Node data update | When a valid Node response is accepted |

The main loop coordinates these operations. SD writes are performed outside the ESP-NOW transport receive callback.

The dashboard and wireless debugging services are updated before transport processing in each loop iteration.

## 6. Remote Node Data Flow

1. ESP-NOW transport receives raw packet bytes.
2. `onTransportDataReceived()` validates the packet size.
3. The callback copies the packet into `WeatherBus::SensorDataPacket`.
4. The callback validates the protocol version.
5. The callback validates the packet type.
6. The callback validates the payload length.
7. Validated packet fields are passed to `PollingEngine::onSensorData()`.
8. `PollingEngine` checks the response against the active request and validates the sensor values.
9. The registered `onValidNodeResponse()` callback stores the accepted reading in the pending logging variables.
10. The callback updates `WebDashboardService` through `webDashboard.setNodeData()`.
11. The main loop processes the pending Node log record.
12. A valid RTC timestamp is obtained.
13. The data logger writes the accepted reading to SD storage.

The transport callback performs packet checks and forwards decoded fields. Polling and response matching are handled by `PollingEngine`.

The dashboard update occurs in the valid Node response callback, while SD writing occurs in the main loop.

## 7. Local Sensor Data Flow

1. The main loop calls `LocalSensorService::readSensors()`.
2. The service obtains local sensor readings and validity flags.
3. The main loop passes the readings and status to `WebDashboardService::setLocalData()`.
4. The main loop reports failed sensor reads through Serial.
5. The main loop prints the local sensor values and validity flags.
6. If the sensor read succeeds, the main loop requests an RTC timestamp.
7. If the timestamp is valid, the data logger writes the local reading to SD storage.

The local sensor is SHT41. It measures temperature and relative humidity; pressure is not measured by this sensor.

Local logging uses Node ID `0`.

Failed local sensor reads are excluded from SD logging.

## 8. RTC Timestamp Handling

`getTimestamp()` returns `false` when:

- The RTC is not in an OK state.
- The supplied buffer is smaller than 20 bytes.

When available, the timestamp is formatted as:

`YYYY-MM-DD HH:MM:SS`

Both local and remote sensor logging require a valid timestamp.

If the timestamp cannot be obtained, the corresponding logging operation is skipped.

### Runtime RTC Monitoring

The main loop checks the RTC approximately every 1000 ms.

- The first RTC update result is reported separately.
- Subsequent update failures are reported through Serial.
- When an update succeeds after a previous failure, the timestamp and recovery status are reported.
- RTC update monitoring does not explicitly halt ESP-NOW polling or the main loop.

The RTC reliability tests and recovery limitations are recorded in the project's testing documentation.

## 9. Remote Node Logging Buffer

The response callback stores the following values:

- Node ID
- Temperature
- Humidity
- Pressure
- Sensor validity flags

It then sets `nodeLogPending` to `true`.

The main loop checks the flag, clears it, obtains an RTC timestamp and performs the SD write.

### Development Consideration

The current implementation has a single pending-record buffer.

If another accepted Node response arrives before the previous pending record is processed, the new values may overwrite the previous values.

The `volatile` qualifier on `nodeLogPending` does not make updates to the complete set of pending variables atomic.

Review this design before increasing Node count, response frequency or logging workload.

## 10. Web Dashboard Integration

`WebDashboardService` is initialized during startup and serviced on every main loop iteration.

### Local Sensor Update

The main loop calls:

`webDashboard.setLocalData(data.temperature, data.humidity, flags, sensorReadOK)`

This passes local temperature, humidity, validity flags and sensor-read status to the dashboard service.

### Remote Node Update

`onValidNodeResponse()` calls:

`webDashboard.setNodeData(nodeId, temperature, humidity, pressure, flags)`

This passes accepted remote Node readings and validity flags to the dashboard service.

### Error Handling

- Dashboard initialization failure is reported through Serial.
- Startup continues if dashboard initialization fails.
- Dashboard update is called on every loop iteration.
- The main application does not explicitly check a return value from `webDashboard.update()`.

Dashboard networking, HTTP handling and presentation details belong to `WebDashboardService`, not this main application.

## 11. Wireless Debug Integration

`WirelessDebugService` provides wireless access to firmware debug output through a Wi-Fi access point and TCP server.

During startup, `wirelessDebug.begin()` is called after Serial initialization.

During normal execution, the main loop calls:

`wirelessDebug.update()`

This services TCP client connections and calls `DebugSerial.update()` through the wireless debugging service.

The wireless debugging service and `DebugSerial` manage the debug connection and transmission buffer. The main application remains responsible for servicing the wireless debugging service.

Wireless debugging is separate from ESP-NOW sensor communication.

## 12. Error Handling and Limitations

- Invalid packet sizes are rejected.
- Unsupported protocol versions are rejected.
- Unexpected packet types are rejected.
- Invalid payload lengths are rejected.
- RTC failures prevent timestamp-dependent logging.
- Failed local sensor reads are reported and excluded from local SD logging.
- Startup transport or peer-registration failure halts execution.
- Wireless debug and dashboard initialization failures do not halt execution.
- The receive callback does not explicitly check whether `data` is null before copying the packet.
- The receive callback does not explicitly validate CRC, reserved fields or unknown validity flag bits.
- A failed SD write is not explicitly reported by the shown logging branches when `logSensorData()` returns `false`.
- The remote Node logging buffer holds only one pending record and may overwrite an unprocessed record.
- The `volatile` pending flag does not make the complete pending record atomic.
- RTC monitoring and recovery reporting are implemented in the main loop, but long-duration reliability depends on separate testing.

These observations describe the supplied implementation. They are not claims that all corresponding failure scenarios have been tested.

## 13. Development Rules

When modifying `src/base_main/main.cpp`:

1. Keep hardware-specific operations inside the appropriate drivers.
2. Keep remote polling logic inside `PollingEngine`.
3. Keep local sensor acquisition inside `LocalSensorService`.
4. Keep dashboard networking and presentation logic inside `WebDashboardService`.
5. Keep wireless TCP connection management inside `WirelessDebugService`.
6. Keep debug buffering and TCP transmission inside `DebugSerial`.
7. Keep SD writes in the main loop rather than the transport callback.
8. Preserve packet validation before forwarding received data.
9. Ensure logging uses valid timestamps and correct sensor flags.
10. Review the single-record pending buffer before expanding to multiple Nodes or increasing response frequency.
11. Preserve RTC monitoring and recovery reporting when changing the main loop.
12. Update this document when initialization, data flow or error handling changes.

## 14. Test Status

Project documentation records successful tests for several integrated modules, including:

- ESP-NOW communication and remote sensor responses.
- Local SHT41 and remote BME280 acquisition.
- RTC disconnect detection, invalid timestamp handling and recovery.
- SD logging and tested runtime recovery scenarios.
- Wireless TCP debug connection and runtime log reception.

The wireless serial debugger remains **CONDITIONAL PASS** because a clean-stream retest after removing temporary diagnostic messages is pending.

RTC long-duration reliability, multi-Node hardware testing and long-duration system stability remain pending according to the project's recorded development status.

This document describes the current source structure and does not establish that every behaviour or failure scenario has been tested against the current commit.

Refer to the project's testing documentation for recorded test results and remaining limitations.

---

# Base Station — `main.cpp`

## 1. Main Flow Tree

```text
BASE STATION : main.cpp
│
├── GLOBAL OBJECTS
│   │
│   ├── NodeManager
│   ├── ESPNowTransport
│   ├── RTCClock
│   ├── WirelessDebugService
│   ├── WebDashboardService
│   ├── PollingEngine
│   │   ├── NodeManager
│   │   └── ESPNowTransport
│   ├── SHT41Source
│   ├── LocalSensorService
│   │   └── SHT41Source
│   ├── SDLogger
│   └── IDataLogger
│       └── SDLogger
│
├── HELPER FUNCTIONS
│   │
│   ├── getTimestamp()
│   │   ├── Check RTC status
│   │   ├── Validate timestamp buffer size
│   │   └── Read and format RTC timestamp
│   │
│   ├── onTransportDataReceived()
│   │   ├── Validate packet size
│   │   ├── Copy packet bytes
│   │   ├── Validate protocol version
│   │   ├── Validate packet type
│   │   ├── Validate payload length
│   │   └── Forward SENSOR_DATA to PollingEngine
│   │
│   ├── onValidNodeResponse()
│   │   ├── Store Node data in pending log buffer
│   │   ├── Set nodeLogPending
│   │   └── Update WebDashboardService
│   │
│   ├── setupPeers()
│   │   ├── Read Node list from NodeManager
│   │   ├── Convert MAC to TransportAddress
│   │   └── Add each Node as ESP-NOW peer
│   │
│   ├── setupTransport()
│   │   ├── Initialize ESPNowTransport
│   │   └── Call setupPeers()
│   │
│   └── setupI2C()
│       ├── Initialize I2C (SDA 8, SCL 9)
│       ├── Set clock to 100 kHz
│       └── Delay 10 ms
│
├── setup()
│   │
│   ├── Initialize Serial at 115200 baud
│   ├── Initialize WirelessDebugService
│   ├── Initialize WebDashboardService
│   ├── setupI2C()
│   ├── Print WeatherBus startup banner
│   ├── nodeManager.begin()
│   ├── localSensorService.begin()
│   ├── dataLogger.begin()
│   ├── rtcClock.begin()
│   │   └── rtcClock.update()
│   │
│   ├── setupTransport()
│   │   ├── transport.begin()
│   │   └── setupPeers()
│   │
│   ├── IF transport initialization or peer setup fails
│   │   └── SYSTEM HALTED
│   │
│   ├── pollingEngine.begin()
│   ├── Register onValidNodeResponse()
│   └── Delay 5 seconds
│
└── loop()
    │
    ├── 1. WEB DASHBOARD
    │   └── webDashboard.update()
    │
    ├── 2. WIRELESS DEBUG
    │   └── wirelessDebug.update()
    │       └── DebugSerial.update()
    │
    ├── 3. TRANSPORT
    │   └── transport.update()
    │
    ├── 4. POLLING
    │   └── pollingEngine.update()
    │
    ├── 5. SD LOGGER
    │   └── sdLogger.update()
    │
    ├── 6. RTC UPDATE (approximately every 1 second)
    │   ├── rtcClock.update()
    │   ├── Report initial update result
    │   ├── Report update failures
    │   └── Report recovery after failure
    │
    ├── 7. NODE SD LOGGING
    │   ├── Check nodeLogPending
    │   ├── Clear pending flag
    │   ├── Get RTC timestamp
    │   ├── dataLogger.logSensorData()
    │   └── Report successful logging
    │
    └── 8. LOCAL SENSOR (approximately every 2 seconds)
        │
        ├── localSensorService.readSensors()
        ├── webDashboard.setLocalData()
        ├── Report sensor-read failure
        ├── Display sensor values and flags
        │
        └── IF sensorReadOK
            ├── Get RTC timestamp
            ├── dataLogger.logSensorData()
            └── Report successful logging
```

## 2. Node Data Flow

```text
NODE DATA FLOW

Sensor Node
    │
    │ ESP-NOW: SENSOR_DATA
    ▼
ESPNowTransport
    │
    ▼
onTransportDataReceived()
    │
    ├── Validate packet size
    ├── Validate protocol version
    ├── Validate packet type
    ├── Validate payload length
    │
    ▼
pollingEngine.onSensorData()
    │
    ├── Validate response
    ├── Update Node status
    └── Invoke valid response callback
            │
            ▼
    onValidNodeResponse()
            │
            ├── Store data in pending log buffer
            ├── Set nodeLogPending
            └── webDashboard.setNodeData()
                        │
                        ▼
                    Web Dashboard

Main loop
    │
    ├── Check nodeLogPending
    ├── Get RTC timestamp
    └── Write data to SD card
```

## 3. Local Sensor Data Flow

```text
LOCAL SENSOR DATA FLOW

SHT41
  │
  ▼
SHT41Source
  │
  ▼
LocalSensorService.readSensors()
  │
  ├── Return sensor values
  ├── Return validity flags
  └── Return read status
          │
          ▼
      Main loop
          │
          ├── webDashboard.setLocalData()
          ├── Display sensor values
          │
          └── IF sensorReadOK
                  │
                  ├── Get RTC timestamp
                  └── Write data to SD card
```

## 4. Wireless Debug Data Flow

```text
WeatherBus Base Application
          │
          ▼
      DebugSerial
          │
          ├──► USB Serial
          │
          └──► Wi-Fi Ring Buffer
                       │
                       ▼
             WirelessDebugService
                       │
                       ├── Manage TCP client
                       └── DebugSerial.update()
                                  │
                                  ▼
                              WiFiClient
                                  │
                                  ▼
                             TCP Port 3333
```

## 5. Web Dashboard Data Flow

```text
WEB DASHBOARD DATA FLOW

Local SHT41
    │
    ▼
LocalSensorService.readSensors()
    │
    ▼
webDashboard.setLocalData()
    │
    ▼
Web Dashboard

Remote Sensor Node
    │
    │ ESP-NOW SENSOR_DATA
    ▼
onTransportDataReceived()
    │
    ▼
PollingEngine.onSensorData()
    │
    ▼
onValidNodeResponse()
    │
    ▼
webDashboard.setNodeData()
    │
    ▼
Web Dashboard
```