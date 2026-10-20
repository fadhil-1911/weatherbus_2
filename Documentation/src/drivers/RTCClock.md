# RTCClock

**Header:** `src/drivers/RTCClock.h`  
**Implementation:** `src/drivers/RTCClock.cpp`

## Purpose

`RTCClock` implements the `IClock` interface to manage date and time using a DS3231 Real-Time Clock (RTC).

It initializes the RTC, retrieves the current time, validates the year, maintains a cached time value, and supports setting the date and time.

## Class Structure

`RTCClock` inherits from `IClock`.

### Public Functions

| Function | Description |
|---|---|
| `begin()` | Initializes the RTC and validates the current time. |
| `update()` | Reads the RTC time, validates the year, and updates the cached time. |
| `isOK()` | Returns the current RTC status. |
| `now()` | Returns the cached `DateTime` value. |
| `setDateTime(dateTime)` | Sets the RTC date and time, then updates and validates the cached time. |

### Private Members

| Member | Description |
|---|---|
| `_rtc` | DS3231 RTC instance. |
| `_now` | Cached date and time from the most recent successful update. |
| `_status` | Indicates whether the most recent initialization or update was successful. Initially `false`. |

## Initialization

### `begin()`

1. Attempts to initialize the DS3231 using `_rtc.begin()`.
2. If initialization fails, sets `_status` to `false` and returns `false`.
3. If initialization succeeds, sets `_status` to `false`.
4. Calls `update()` to validate the RTC time and update the cached value.

Returns `true` only when `update()` succeeds.

## Time Update

### `update()`

Reads the current date and time from the RTC.

The year must be within the inclusive range of 2020–2100.

- If the year is outside this range, `_status` becomes `false` and the function returns `false`.
- If the year is valid, the time is copied into `_now`, `_status` becomes `true`, and the function returns `true`.

The implementation validates only the year range.

## RTC Status

### `isOK()`

Returns `_status`, indicating whether the most recent initialization or update operation succeeded.

### `now()`

Returns the cached `_now` value.

This function does not read the RTC directly or independently verify the current status.

## Setting Date and Time

### `setDateTime(dateTime)`

1. Calls `_rtc.adjust(dateTime)` to set the RTC date and time.
2. Calls `update()` to read the time again and validate the year.
3. Returns the result of `update()`.

The implementation does not separately verify the result of `_rtc.adjust()`.

## Dependencies

- `RTCClock.h`
- `IClock.h`
- RTClib types `RTC_DS3231` and `DateTime`
- Arduino `Wire` interface

## Behavioural Notes and Limitations

- The RTC must initialize successfully before `begin()` can return `true`.
- The accepted year range is 2020–2100, inclusive.
- An invalid year sets `_status` to `false`.
- When `update()` fails due to an invalid year, `_now` is not cleared; `now()` may therefore return a previously cached value.
- The implementation does not explicitly check RTC oscillator-stop status.
- The implementation does not validate the full date and time beyond the year range.
- The class does not schedule periodic updates; the caller determines when `update()` is called.

## Test Status

**Code review:** Initialization, year validation, cached time handling, status reporting, and date/time adjustment are documented from the active implementation.

RTC hardware and recovery test results should be recorded separately.