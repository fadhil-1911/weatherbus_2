# Sensor Node — Main Application

## 1. Purpose

Implements the main application entry point for a WeatherBus Sensor Node. It initializes I2C, configures the enabled sensors, initializes the communication service, and processes incoming communication requests through the service update loop.

Source: `src/node_main/main.cpp`

## 2. Main Components

| Component | Responsibility |
|---|---|
| `SHT41Source` | Reads temperature and humidity from the SHT41 sensor |
| `BME280Source` | Reads temperature, humidity, and pressure from the BME280 sensor |
| `ESPNowTransport` | Provides ESP-NOW communication |
| `NodeCommunicationService` | Handles Node communication and sensor-data responses |

The communication service receives references to the transport and both sensor sources.

## 3. I2C Initialization

### `setupI2C()`

1. Initializes I2C using `I2C_SDA` and `I2C_SCL`.
2. Sets the bus clock using `I2C_CLOCK`.
3. Waits 10 ms.
4. Prints an initialization message.

The function currently returns `true` unconditionally; it does not verify whether I2C initialization succeeded.

## 4. Startup Sequence

The `setup()` function performs the following operations:

1. Initializes Serial at 115200 baud.
2. Calls `setupI2C()`.
3. Prints the WeatherBus version, Node ID, and sensor configuration.
4. Initializes SHT41 if `ENABLE_SHT41` is true.
5. Initializes BME280 if `ENABLE_BME280` is true.
6. Builds a `TransportAddress` using `baseMac`.
7. Calls `nodeCommunicationService.begin(baseAddress)`.
8. Halts the system if the communication service fails to initialize.
9. Prints the Node-ready messages after successful initialization.

## 5. Sensor Configuration

Sensor selection is controlled by constants in `node_settings.h`.

- `ENABLE_SHT41` determines whether SHT41 initialization is attempted.
- `ENABLE_BME280` determines whether BME280 initialization is attempted.

If an enabled sensor fails to initialize, the application logs an error and continues startup. The actual sensor availability and subsequent reading behaviour depend on the sensor drivers and `NodeCommunicationService`.

## 6. Base Station Address

The application creates a `TransportAddress` from `baseMac`.

- The address length is set to `sizeof(baseMac)`.
- The MAC address bytes are copied into `baseAddress.data`.
- The resulting address is passed to `nodeCommunicationService.begin()`.

The Base Station MAC address must match the intended communication destination.

## 7. Main Loop

The `loop()` function calls:

`nodeCommunicationService.update()`

The application does not directly poll sensors or process received packets in `loop()`. These responsibilities are delegated to the communication service and transport implementation.

According to the source comments, the Node responds when a valid `DATA_REQUEST` is received.

## 8. Failure Behaviour

| Condition | Behaviour |
|---|---|
| `setupI2C()` returns false | Startup halts |
| Enabled SHT41 fails to initialize | Logs an error and continues |
| Enabled BME280 fails to initialize | Logs an error and continues |
| `nodeCommunicationService.begin()` fails | Logs `SYSTEM HALTED` and halts |
| Sensor disabled by configuration | Its `begin()` function is not called |

Because `setupI2C()` currently returns `true` unconditionally, its failure branch does not detect actual I2C initialization failure.

## 9. Implementation Notes

- The Node ID and sensor-selection settings are defined in `node_settings.h`.
- The main application creates the sensor and transport objects globally.
- Sensor reading and request-response handling are delegated to `NodeCommunicationService`.
- The main loop contains no explicit delay or polling interval.
- The application does not directly implement packet validation or sensor-data transmission in this file.

## 10. Development Constraints

- Keep `NODE_ID` consistent with the Base Station configuration.
- Verify that `baseMac` matches the Base Station.
- Ensure sensor-selection settings match the intended hardware configuration.
- Review I2C initialization error handling if actual bus failures must be detected.
- Test startup behaviour when sensors or ESP-NOW communication are unavailable.

## 11. Test Status

This file defines application startup and loop behaviour. It does not, by itself, establish that every sensor configuration, startup failure, or communication scenario has passed testing.

Record verified hardware and integration results separately.


TREE FLOW

Sensor Node - Main Application
├── Global Objects
│   ├── SHT41Source sht41Source
│   ├── BME280Source bme280Source
│   │   └── Initialize with BME280_I2C_ADDRESS
│   ├── ESPNowTransport transport
│   └── NodeCommunicationService
│       ├── transport
│       ├── bme280Source
│       └── sht41Source
│
├── setupI2C()
│   ├── Initialize I2C using I2C_SDA and I2C_SCL
│   ├── Set I2C clock speed using I2C_CLOCK
│   ├── Delay 10 ms
│   ├── Print "I2C bus ready"
│   └── return true
│
├── setup()
│   ├── Initialize Serial (115200 baud)
│   ├── Delay 1000 ms
│   ├── Call setupI2C()
│   │   └── Failed
│   │       ├── Print initialization error
│   │       └── Halt system
│   ├── Print WeatherBus V1.0 information
│   ├── Print NODE_ID
│   ├── Print sensor configuration
│   │
│   ├── Is ENABLE_SHT41 == true?
│   │   ├── Yes → Call sht41Source.begin()
│   │   │   └── Failed
│   │   │       ├── Print "SHT41 FAILED"
│   │   │       └── Continue without temperature/humidity
│   │   └── No → Skip SHT41 initialization
│   │
│   ├── Is ENABLE_BME280 == true?
│   │   ├── Yes → Call bme280Source.begin()
│   │   │   └── Failed
│   │   │       ├── Print "BME280 FAILED"
│   │   │       └── Continue without pressure
│   │   └── No → Skip BME280 initialization
│   │
│   ├── Prepare Base Station address
│   │   ├── Set address length = sizeof(baseMac)
│   │   └── Copy baseMac into TransportAddress
│   │
│   ├── Call nodeCommunicationService.begin(baseAddress)
│   │   ├── Failed
│   │   │   ├── Print "SYSTEM HALTED"
│   │   │   └── Halt system
│   │   └── Successful
│   │       ├── Print "ESP-NOW ready"
│   │       └── Print "Node ready"
│   └── End setup
│
└── loop()
    ├── No periodic sensor polling
    ├── Call nodeCommunicationService.update()
    └── Respond only when a valid DATA_REQUEST is received