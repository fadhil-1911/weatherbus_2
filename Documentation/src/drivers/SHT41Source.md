# SHT41Source

**Header:** `src/drivers/SHT41Source.h`  
**Implementation:** `src/drivers/SHT41Source.cpp`

## Purpose

`SHT41Source` implements the `ISensorSource` interface to read temperature and relative humidity from an SHT41 sensor using I2C.

The driver sends the high-precision measurement command, validates the returned data using CRC-8, converts raw values into physical measurements, and reports measurement validity through `SensorReading`.

## Class Structure

`SHT41Source` inherits from `ISensorSource`.

### Public Functions

| Function | Description |
|---|---|
| `SHT41Source()` | Constructs the driver with the sensor marked as unavailable. |
| `begin()` | Initializes communication with the SHT41 sensor. |
| `read(SensorReading& reading)` | Reads temperature and humidity and updates the reading validity flags. |

### Private Functions

| Function | Description |
|---|---|
| `setupSHT41()` | Sends the high-precision measurement command to check communication. |
| `readSHT41(...)` | Retrieves sensor data, validates CRC, and converts raw values. |
| `sht41Crc8(...)` | Calculates CRC-8 using polynomial `0x31`. |

### Private Member

| Member | Description |
|---|---|
| `available` | Indicates whether the initial sensor setup succeeded. |

## Sensor Configuration

| Parameter | Value |
|---|---|
| I2C address | `0x44` |
| Measurement command | `0xFD` |
| Measurement mode | High precision |
| Expected response length | 6 bytes |
| CRC polynomial | `0x31` |
| CRC initial value | `0xFF` |
| Measurement delay | 10 ms |

## Initialization

### `begin()`

1. Prints the driver initialization message.
2. Calls `setupSHT41()`.
3. Stores the result in `available`.
4. If setup fails, prints a warning and returns `false`.
5. Returns `true` if setup succeeds.

A failed initialization does not stop the application through this driver; the caller can decide how to proceed.

### `setupSHT41()`

1. Starts an I2C transmission to address `0x44`.
2. Sends command `0xFD`.
3. Checks the result of `Wire.endTransmission()`.
4. Returns `false` if the transmission fails.
5. Waits 10 ms after a successful transmission.
6. Returns `true`.

This function checks the command transmission but does not retrieve or validate a complete measurement response.

## Measurement Process

### `readSHT41(...)`

The function performs the following operations:

1. Sends the high-precision measurement command.
2. Returns `false` if the I2C transmission fails.
3. Waits 10 ms for the measurement.
4. Requests six bytes from the sensor.
5. Returns `false` if the response length is not exactly six bytes.
6. Reads the six response bytes.
7. Validates the temperature CRC.
8. Validates the humidity CRC.
9. Converts the raw temperature and humidity values.
10. Clamps humidity to the range 0–100%.
11. Returns `true` when the complete measurement succeeds.

### Response Data Layout

| Byte(s) | Content |
|---|---|
| `data[0]`, `data[1]` | Raw temperature |
| `data[2]` | Temperature CRC |
| `data[3]`, `data[4]` | Raw humidity |
| `data[5]` | Humidity CRC |

Temperature and humidity are validated independently using the CRC bytes associated with each measurement.

## CRC-8 Validation

### `sht41Crc8(...)`

The CRC calculation uses:

- Initial value: `0xFF`
- Polynomial: `0x31`
- Eight processing iterations per input byte

The driver rejects the measurement if either the temperature or humidity CRC does not match the received CRC byte.

## Measurement Conversion

### Temperature

The raw 16-bit temperature value is converted using:

`temperature = -45.0 + 175.0 × (rawTemperature / 65535.0)`

The resulting value is expressed in degrees Celsius.

### Relative Humidity

The raw 16-bit humidity value is converted using:

`humidity = -6.0 + 125.0 × (rawHumidity / 65535.0)`

The resulting value is clamped to the range `0.0–100.0` percent.

## Sensor Reading Interface

### `read(SensorReading& reading)`

Before reading the sensor, the function:

- Sets temperature and humidity to `0.0`.
- Sets `temperatureValid` and `humidityValid` to `false`.

It then checks whether the sensor is marked as available.

- If `available` is `false`, it returns `false`.
- If the measurement fails, it returns `false`.
- If the measurement succeeds, it sets both validity flags to `true` and returns `true`.

The driver does not set pressure or a pressure-validity flag.

## Dependencies

- `SHT41Source.h`
- `ISensorSource.h`
- Arduino framework
- `Wire` I2C interface

## Behavioural Notes and Limitations

- The driver uses a fixed I2C address of `0x44`.
- The measurement command is fixed to `0xFD`.
- The driver does not implement an explicit sensor recovery routine.
- If initialization fails, `available` remains `false` until `begin()` is called again successfully.
- A failed measurement leaves the reading values initialized to zero and the validity flags set to `false`.
- Temperature and humidity are marked valid together only after the complete measurement succeeds.
- The driver does not perform additional range or finite-value checks after conversion.
- The driver does not provide pressure measurements.

## Test Status

**Code review:** Initialization, I2C communication, response-length checking, CRC validation, measurement conversion, humidity clamping, and validity-flag handling are documented from the provided implementation.

Hardware testing should verify sensor detection, valid measurements, CRC failures, incomplete I2C responses, and recovery after communication errors.




TREE FLOW
```text
SHT41Source
├── Constructor()
│   └── available = false
│
├── sht41Crc8(data, length)
│   ├── Initialize CRC = 0xFF
│   ├── For each data byte
│   │   ├── XOR byte with CRC
│   │   └── Process 8 bits
│   │       ├── MSB set → Shift left and XOR 0x31
│   │       └── MSB not set → Shift left
│   └── return CRC
│
├── setupSHT41()
│   ├── Start I2C transmission to address 0x44
│   ├── Send high-precision measurement command (0xFD)
│   ├── Transmission failed?
│   │   └── Yes → Print error → return false
│   ├── Delay 10 ms
│   ├── Print communication OK
│   └── return true
│
├── begin()
│   ├── Print "SHT41 Source"
│   ├── Call setupSHT41()
│   ├── Store result in available
│   ├── If unavailable
│   │   └── Print unavailable message
│   └── return available
│
├── readSHT41(temperature, humidity)
│   ├── Send measurement command (0xFD)
│   │   └── Transmission failed → return false
│   ├── Delay 10 ms
│   ├── Request 6 bytes from SHT41
│   │   └── Received bytes != 6 → return false
│   ├── Read 6 data bytes
│   ├── Validate temperature CRC
│   │   └── Failed → Print error → return false
│   ├── Validate humidity CRC
│   │   └── Failed → Print error → return false
│   ├── Extract raw temperature and humidity values
│   ├── Convert raw temperature
│   │   └── T = -45 + 175 × (rawTemperature / 65535)
│   ├── Convert raw humidity
│   │   └── RH = -6 + 125 × (rawHumidity / 65535)
│   ├── Clamp humidity to 0–100%
│   └── return true
│
└── read(reading)
    ├── Initialize temperature and humidity to 0
    ├── Set temperatureValid = false
    ├── Set humidityValid = false
    ├── Is available == false?
    │   └── Yes → return false
    ├── Call readSHT41()
    │   └── Failed → return false
    ├── Set temperatureValid = true
    ├── Set humidityValid = true
    └── return true
```