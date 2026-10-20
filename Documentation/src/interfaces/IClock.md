# IClock Interface

## 1. Purpose

Defines an abstract interface for clock operations in WeatherBus. It provides a common contract for initializing the clock, updating its state, checking its status, retrieving the current time, and setting the date and time.

Source: `src/interfaces/IClock.h`

## 2. Interface Methods

| Method | Return Type | Purpose |
|---|---|---|
| `begin()` | `bool` | Initializes the clock |
| `update()` | `bool` | Updates the clock state |
| `isOK()` | `bool` | Reports whether the clock is in a valid state |
| `now()` | `DateTime` | Returns the current or cached date and time |
| `setDateTime(const DateTime& dateTime)` | `bool` | Sets the date and time |

All methods are pure virtual and must be implemented by a concrete class.

## 3. Dependencies

- `RTClib.h` provides the `DateTime` type used by the interface.

## 4. Design

`IClock` separates clock operations from the underlying RTC implementation.

Classes that implement this interface must provide the declared methods. This allows higher-level code to depend on the interface rather than directly on a specific clock driver.

The virtual destructor allows derived objects to be safely destroyed through an `IClock` pointer.

## 5. Implementation Notes

- This file declares an interface only; it does not communicate with RTC hardware.
- Time validation, status management, and recovery behaviour depend on the implementing class.
- The interface does not specify the criteria for a valid timestamp.
- The interface does not prescribe how often `update()` should be called.

## 6. Development Constraints

- Concrete implementations must implement all five methods.
- Keep method signatures consistent across implementations.
- Callers should check `isOK()` and handle failed operations where appropriate.
- Document implementation-specific behaviour in the corresponding driver documentation.

## 7. Test Status

This header defines the interface contract. It does not, by itself, establish hardware functionality or test results. Verify behaviour through the concrete implementation and its tests.