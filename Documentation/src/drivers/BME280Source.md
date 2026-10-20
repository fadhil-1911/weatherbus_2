# BME280Source

**Header:** `src/drivers/BME280Source.h`  
**Implementation:** `src/drivers/BME280Source.cpp`

## Purpose

`BME280Source` implements the `ISensorSource` interface to acquire temperature, humidity, and atmospheric pressure measurements from a BME280 sensor.

It validates sensor readings, attempts sensor recovery when any measurement is invalid, and applies calibration offsets to valid temperature and humidity readings.

## Class Structure

`BME280Source` inherits from `ISensorSource`.

### Public Functions

| Function | Description |
|---|---|
| `BME280Source(uint8_t address = 0x76)` | Constructs the sensor source using the specified I2C address. The default address is `0x76`. |
| `begin()` | Initializes communication with the BME280 sensor. |
| `read(SensorReading& reading)` | Reads, validates, calibrates, and returns sensor measurements with validity flags. |

### Private Functions

| Function | Description |
|---|---|
| `recover()` | Attempts to reinitialize the BME280 sensor. |
| `isTemperatureValid(float value)` | Checks whether temperature is finite and within the accepted range. |
| `isHumidityValid(float value)` | Checks whether humidity is finite and within the accepted range. |
| `isPressureValid(float value)` | Checks whether pressure is finite and within the accepted range. |

### Private Members

| Member | Description |
|---|---|
| `_bme` | Adafruit BME280 sensor instance. |
| `_address` | Configured I2C address of the sensor. |

## Initialization

### `begin()`

Calls the Adafruit BME280 library to initialize the sensor using `_address` and the global `Wire` interface.

- Returns `true` and prints a success message when initialization succeeds.
- Returns `false` and prints an error message when initialization fails.

## Sensor Reading

### `read(SensorReading& reading)`

The function follows these steps:

1. Resets the output structure using `SensorReading{}`.
2. Reads temperature in degrees Celsius.
3. Reads relative humidity in percent.
4. Reads pressure and converts it from pascals to hectopascals (hPa).
5. Validates each measurement independently.
6. Attempts sensor recovery if any measurement is invalid.
7. Re-reads and validates all measurements after successful recovery.
8. Applies calibration offsets to valid temperature and humidity readings.
9. Stores measurements and their validity flags in `SensorReading`.

The function returns `false` if recovery fails or all three measurements remain invalid after recovery.

Otherwise, it returns `true`, even if one or two measurements remain invalid.

## Measurement Validation

| Measurement | Accepted Range | Additional Requirement |
|---|---|---|
| Temperature | Greater than -40 °C and less than 85 °C | Must be finite. |
| Humidity | 0–100% inclusive | Must be finite. |
| Pressure | 300–1100 hPa inclusive | Must be finite. |

Each measurement is validated independently.

## Calibration

The implementation defines these calibration offsets:

| Measurement | Offset | Application |
|---|---:|---|
| Temperature | -0.68 °C | Applied only when temperature is valid. |
| Humidity | +8.98 percentage points | Applied only when humidity is valid. |

Humidity is constrained to the range 0–100% after applying its offset.

Pressure has no calibration offset in this implementation.

## Recovery Behaviour

When any measurement is invalid, `read()` calls `recover()`.

The recovery function attempts to reinitialize the sensor using the configured I2C address.

- If recovery fails, `read()` returns `false`.
- If recovery succeeds, all three measurements are read again and revalidated.
- If all three measurements remain invalid, `read()` returns `false`.
- If at least one measurement is valid, processing continues.

The implementation does not return `false` merely because one or two measurements remain invalid after recovery.

## Output Handling

Valid measurements are stored in `SensorReading`, with the corresponding validity flags set to `true`.

Invalid measurements are stored as `0.0f`, with their validity flags set to `false`.

Consumers should check the validity flags before using the measurement values.

## Dependencies

- `BME280Source.h`
- `ISensorSource.h`
- Adafruit BME280 library
- Arduino `Wire`
- Arduino `constrain()`
- `isfinite()` and `NAN` support from the math library

## Debugging and Test Hooks

The implementation includes commented-out test hooks for simulating:

- Invalid temperature.
- Invalid humidity.
- Invalid pressure.
- All three measurements being invalid.

These hooks are disabled in the current code.

Serial output reports raw measurements, invalid readings, and recovery results.

## Behavioural Notes and Limitations

- A successful return from `read()` does not guarantee that every measurement is valid.
- Consumers must use the validity flags to identify usable measurements.
- Recovery reinitializes the sensor but does not implement a retry loop.
- Calibration offsets are hard-coded constants.
- The source does not implement sensor polling intervals; callers control when `read()` is invoked.

## Test Status

**Code review:** Class structure, measurement validation, calibration, recovery, and output handling are documented from the provided source.

Hardware test results should be recorded separately.