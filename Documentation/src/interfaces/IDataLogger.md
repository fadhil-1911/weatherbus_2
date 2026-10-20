# IDataLogger Interface

## 1. Purpose

Defines an abstract interface for data logging in WeatherBus. It establishes a common contract for initializing the logger, checking its readiness, and recording sensor data.

Source: `src/interfaces/IDataLogger.h`

## 2. Interface Methods

| Method | Return Type | Purpose |
|---|---|---|
| `begin()` | `bool` | Initializes the logger |
| `isReady()` | `bool` | Reports whether the logger is ready |
| `logSensorData(...)` | `bool` | Records a sensor data entry |

### `logSensorData()` Parameters

| Parameter | Type | Description |
|---|---|---|
| `timestamp` | `const char*` | Timestamp associated with the record |
| `nodeId` | `uint8_t` | Sensor Node identifier |
| `temperature` | `float` | Temperature value |
| `humidity` | `float` | Relative humidity value |
| `pressure` | `float` | Atmospheric pressure value |
| `flags` | `uint8_t` | Flags associated with the sensor readings |

All methods are pure virtual and must be implemented by a concrete logger class.

## 3. Dependencies

- `Arduino.h` provides Arduino-related types, including `uint8_t`.

## 4. Design

`IDataLogger` separates data-logging operations from the underlying storage implementation.

Higher-level code can use this interface without depending directly on a specific logger class, such as `SDLogger`.

The virtual destructor allows derived objects to be safely destroyed through an `IDataLogger` pointer.

## 5. Implementation Notes

- This header defines an interface only; it does not implement storage or file operations.
- The interface does not specify a file format, storage medium, retry policy, or recovery mechanism.
- The meaning of `flags` and the handling of invalid measurements depend on the implementation and calling code.
- The return value of `logSensorData()` indicates the result reported by the concrete implementation.

## 6. Development Constraints

- Concrete implementations must implement all three methods.
- Keep method signatures consistent across implementations.
- Callers should check return values and use `isReady()` where appropriate.
- Document implementation-specific logging and recovery behaviour in the corresponding driver documentation.

## 7. Test Status

This header defines the logging interface contract. It does not, by itself, establish storage functionality or test results. Verify behaviour through the concrete implementation and its tests.