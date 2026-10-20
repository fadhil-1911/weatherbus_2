# ISensorSource Interface

## 1. Purpose

Defines the interface for sensor sources in WeatherBus. It establishes a common contract for initializing a sensor source and reading environmental measurements into a `SensorReading` structure.

Source: `src/interfaces/ISensorSource.h`

## 2. SensorReading Structure

`SensorReading` stores sensor measurements and their individual validity states.

| Field | Type | Description |
|---|---|---|
| `temperature` | `float` | Temperature value in °C |
| `humidity` | `float` | Relative humidity in % |
| `pressure` | `float` | Atmospheric pressure |
| `temperatureValid` | `bool` | Indicates whether temperature is valid |
| `humidityValid` | `bool` | Indicates whether humidity is valid |
| `pressureValid` | `bool` | Indicates whether pressure is valid |

### Default Initialization

The constructor initializes all measurement values to `0.0f` and all validity flags to `false`.

A zero value does not necessarily represent a valid measurement. Consumers should check the corresponding validity flag.

## 3. Interface Methods

| Method | Return Type | Purpose |
|---|---|---|
| `begin()` | `bool` | Initializes the sensor source |
| `read(SensorReading& reading)` | `bool` | Reads measurements into the supplied structure |

Both methods are pure virtual and must be implemented by a concrete sensor source class.

## 4. Dependencies

- `Arduino.h` provides Arduino-related definitions used by the interface.

## 5. Design

`ISensorSource` separates sensor-reading operations from specific sensor hardware implementations.

Higher-level services can interact with different sensor sources through the same interface without depending directly on a particular driver.

The virtual destructor allows derived objects to be safely destroyed through an `ISensorSource` pointer.

## 6. Implementation Notes

- This header defines the data structure and interface only; it does not communicate with sensor hardware.
- The interface supports temperature, humidity, and pressure fields.
- Individual validity flags allow implementations to represent partially valid measurements.
- The meaning of the `begin()` and `read()` return values depends on the concrete implementation.
- The interface does not define sensor selection, calibration, recovery, or fallback behaviour.

## 7. Development Constraints

- Concrete sensor sources must implement both `begin()` and `read()`.
- Keep measurement units consistent across implementations.
- Set validity flags accurately for each measurement.
- Callers should check both the `read()` return value and the relevant validity flags.
- Document sensor-specific behaviour in the corresponding driver documentation.

## 8. Test Status

This header defines the sensor-source contract and reading structure. It does not, by itself, establish sensor functionality or test results. Verify behaviour through concrete driver implementations and their tests.