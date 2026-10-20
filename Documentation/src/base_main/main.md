# Base Station — Main Application

## 1. Purpose

Entry point for the WeatherBus Base Station.

Coordinates sensor acquisition, ESP-NOW communication, Node polling,
RTC timestamp management and SD data logging.

**Source:** `src/base_main/main.cpp`

## 2. Responsibilities

- Initialize I2C and system modules.
- Initialize the local SHT41 sensor service.
- Initialize the SD logger and RTC clock.
- Initialize ESP-NOW transport and register configured Node peers.
- Receive and validate sensor data packets.
- Coordinate remote Node polling through `PollingEngine`.
- Log local and remote sensor readings with RTC timestamps.
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
| `weatherbus_protocol.h` | Defines packet structures and protocol constants |
| `node_config.h` | Provides Base Station Node configuration |

## 4. Initialization Sequence

The `setup()` function performs these operations:

1. Start Serial communication at 115200 baud.
2. Initialize I2C.
3. Initialize `NodeManager`.
4. Initialize `LocalSensorService`.
5. Initialize the data logger.
6. Initialize and update the RTC.
7. Initialize ESP-NOW transport.
8. Register configured Node peers.
9. Initialize `PollingEngine`.
10. Register the valid Node response callback.
11. Wait five seconds before normal loop execution.

### Initialization Failure Behaviour

| Component | Current Behaviour |
|---|---|
| Local sensor service | Prints an error and continues |
| SD logger | Prints a warning and continues |
| RTC | Reports initialization/update status |
| ESP-NOW transport or peer setup | Halts the system if setup fails |

Review this behaviour before changing startup reliability requirements.

## 5. Main Loop

The `loop()` function periodically services the following components:

| Operation | Frequency / Trigger |
|---|---|
| `transport.update()` | Every loop iteration |
| `pollingEngine.update()` | Every loop iteration |
| `sdLogger.update()` | Every loop iteration |
| `rtcClock.update()` | Approximately every 1000 ms |
| Local sensor read | Approximately every 2000 ms |
| Remote Node logging | When `nodeLogPending` is set |

The main loop is responsible for coordinating these operations without
placing SD writes directly inside the transport receive callback.

## 6. Remote Node Data Flow

1. ESP-NOW receives raw packet bytes.
2. `onTransportDataReceived()` validates packet size.
3. The callback validates protocol version and packet type.
4. It validates the payload length.
5. Validated fields are passed to `PollingEngine::onSensorData()`.
6. `PollingEngine` checks whether the response matches the active request
   and validates the sensor values.
7. The registered `onValidNodeResponse()` callback stores the accepted
   reading in the pending logging variables.
8. The main loop obtains an RTC timestamp.
9. The data logger writes the reading to SD storage.

The transport callback performs packet checks and forwards the decoded
fields. Polling and response matching are handled by `PollingEngine`.

## 7. Local Sensor Data Flow

1. The main loop calls `LocalSensorService::readSensors()`.
2. The service obtains local sensor readings.
3. The service returns sensor values and validity flags.
4. The main loop prints the results to Serial.
5. If the sensor read succeeds and the RTC timestamp is valid, the data
   is passed to `IDataLogger::logSensorData()`.

The local sensor is SHT41. Its measurements are temperature and relative
humidity; pressure is not measured by this sensor.

## 8. RTC Timestamp Handling

`getTimestamp()` returns `false` when:

- The RTC is not in an OK state.
- The supplied buffer is smaller than 20 bytes.

When available, the timestamp is formatted as:

`YYYY-MM-DD HH:MM:SS`

Both local and remote sensor logging require a valid timestamp.

If the timestamp cannot be obtained, the corresponding log operation
is skipped.

## 9. Remote Node Logging Buffer

The response callback stores the following values:

- Node ID
- Temperature
- Humidity
- Pressure
- Sensor validity flags

It then sets `nodeLogPending` to `true`.

The main loop processes this pending record and performs the SD write.

### Development Consideration

The current implementation has a single pending-record buffer.

If another accepted Node response arrives before the previous pending
record is processed, the new values may overwrite the previous values.

Review this design before introducing multiple Nodes, higher response
rates or potentially blocking logging operations.

The `volatile` qualifier on `nodeLogPending` does not make updates to
the complete set of pending variables atomic.

## 10. Error Handling and Limitations

- Invalid packet sizes are rejected.
- Unsupported protocol versions are rejected.
- Unexpected packet types are rejected.
- Invalid payload lengths are rejected.
- RTC failures prevent timestamp-dependent logging.
- Failed local sensor reads are reported through Serial.
- Startup transport or peer-registration failure halts execution.
- The receive callback currently does not explicitly check whether
  `data` is null before copying the packet.
- The receive callback does not explicitly validate CRC, reserved fields
  or unknown validity flag bits.
- A failed SD write is not explicitly reported by the shown logging
  branches when `logSensorData()` returns `false`.

These observations describe the current implementation. They are not
claims that all corresponding failure scenarios have been tested.

## 11. Development Rules

When modifying `base_main/main.cpp`:

1. Keep hardware-specific operations inside the appropriate drivers.
2. Keep remote polling logic inside `PollingEngine`.
3. Keep local sensor acquisition inside `LocalSensorService`.
4. Keep SD writes in the main loop rather than the transport callback.
5. Preserve packet validation before forwarding received data.
6. Ensure logging uses valid timestamps and correct sensor flags.
7. Review the single-record pending buffer before expanding to multiple
   Nodes or increasing response frequency.
8. Update this document when initialization, data flow or error handling
   changes.

## 12. Test Status

Historical project notes report successful tests for several integrated
modules, including RTC recovery, SD recovery and remote sensor communication.

This file alone does not establish that every behaviour described above
has been tested against the current commit.

Refer to the project's testing documentation for the recorded test
results and remaining limitations.




TREE FLOW

TREE FLOW

BASE STATION : main.cpp
│
├── GLOBAL OBJECTS
│   │
│   ├── NodeManager
│   ├── ESPNowTransport
│   ├── RTCClock
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
│   │   └── Read RTC time and format timestamp
│   │
│   ├── onTransportDataReceived()
│   │   ├── Validate packet size
│   │   ├── Validate protocol version
│   │   ├── Validate packet type
│   │   ├── Validate payload length
│   │   └── Forward SENSOR_DATA to PollingEngine
│   │
│   ├── onValidNodeResponse()
│   │   └── Store Node data in pending log buffer
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
│       └── Set clock to 100 kHz
│
├── setup()
│   │
│   ├── Initialize Serial
│   ├── setupI2C()
│   ├── nodeManager.begin()
│   ├── localSensorService.begin()
│   ├── dataLogger.begin()
│   ├── rtcClock.begin()
│   │   └── rtcClock.update()
│   ├── setupTransport()
│   │   ├── transport.begin()
│   │   └── setupPeers()
│   │
│   ├── IF transport initialization fails
│   │   └── SYSTEM HALTED
│   │
│   ├── pollingEngine.begin()
│   ├── Register onValidNodeResponse()
│   └── Delay 5 seconds
│
└── loop()
    │
    ├── 1. TRANSPORT
    │   └── transport.update()
    │
    ├── 2. POLLING
    │   └── pollingEngine.update()
    │
    ├── 3. SD LOGGER
    │   └── sdLogger.update()
    │
    ├── 4. RTC UPDATE (every 1 second)
    │   ├── rtcClock.update()
    │   ├── Detect initial RTC status
    │   ├── Report update failure
    │   └── Report recovery after failure
    │
    ├── 5. NODE SD LOGGING
    │   ├── Check nodeLogPending
    │   ├── Get RTC timestamp
    │   ├── dataLogger.logSensorData()
    │   └── Clear pending flag
    │
    └── 6. LOCAL SENSOR (every 2 seconds)
        │
        ├── localSensorService.readSensors()
        ├── Display sensor values and flags
        │
        └── IF sensorReadOK
            ├── Get RTC timestamp
            ├── dataLogger.logSensorData()
            └── Log local sensor data to SD



NODE DATA FLOW


Sensor Node
    │
    │ ESP-NOW: SENSOR_DATA
    ▼
transport.begin(callback)
    │
    ▼
onTransportDataReceived()
    │
    ├── Validate packet
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
            └── Store data in pending log buffer
                        │
                        ▼
                    loop()
                        │
                        ├── Get RTC timestamp
                        └── Write data to SD card