# SDLogger

**Header:** `src/base/SDLogger.h`  
**Implementation:** `src/base/SDLogger.cpp`

## Purpose

`SDLogger` implements the `IDataLogger` interface to store environmental sensor readings in a CSV file on an SD card.

It handles SD card initialization, CSV file creation, sensor data logging, write verification, and automatic recovery attempts when the SD card becomes unavailable.

## Class Structure

`SDLogger` inherits from `IDataLogger`.

### Public Functions

| Function | Description |
|---|---|
| `SDLogger(uint8_t csPin)` | Constructs the logger with the SD card chip-select pin. |
| `begin()` | Initializes the SD card and ensures the CSV file exists. |
| `update()` | Attempts SD card recovery when logging is unavailable. |
| `isReady()` | Returns the current SD logger status. |
| `logSensorData(...)` | Writes one sensor record to the CSV file. |

### Private Members

| Member | Description |
|---|---|
| `LOG_FILE` | CSV filename: `log_wb2.csv`. |
| `RETRY_INTERVAL_MS` | Recovery retry interval of 5000 ms. |
| `sd` | `SdFat` instance for SD card operations. |
| `logFile` | File handle used to access the CSV file. |
| `csPin` | SD card chip-select pin supplied to the constructor. |
| `sdStatus` | Indicates whether the SD logger is ready. |
| `lastRetryMs` | Timestamp of the most recent recovery attempt. |
| `createLogFile()` | Creates the CSV file and writes its header. |

## Initialization

### `begin()`

1. Prints an initialization message.
2. Sets `sdStatus` to `false`.
3. Closes the file handle if it is already open.
4. Initializes SPI using `SPI.begin(4, 5, 6, 10)`.
5. Initializes the SD card using the configured chip-select pin and an SPI clock of 10 MHz.
6. If the card initialization fails, returns `false`.
7. Checks whether `log_wb2.csv` exists:
   - If it does not exist, calls `createLogFile()`.
   - If it exists, keeps the existing file.
8. Sets `sdStatus` to `true` and returns `true`.

If SD initialization or CSV creation fails, the function returns `false`.

## CSV File Format

The file is named `log_wb2.csv`.

The header contains six columns:

```csv
Timestamp,Node_ID,Temperature_C,Humidity_PCT,Pressure_hPa,Flags



TREE FLOW
```text
SDLogger
├── Constructor(csPin)
│   ├── Store CS pin
│   ├── sdStatus = false
│   └── lastRetryMs = 0
│
├── begin()
│   ├── Print initialization message
│   ├── sdStatus = false
│   ├── Close previous file handle if open
│   ├── Initialize SPI (SCK=4, MISO=5, MOSI=6, CS=10)
│   ├── Initialize SD card
│   │   └── Failed → Print error → return false
│   ├── Check whether LOG_FILE exists
│   │   ├── No → createLogFile()
│   │   │   └── Failed → return false
│   │   └── Yes → Print file found message
│   ├── sdStatus = true
│   └── return true
│
├── createLogFile()
│   ├── Open LOG_FILE using FILE_WRITE
│   ├── Failed to open
│   │   ├── sdStatus = false
│   │   └── return false
│   ├── Write CSV header
│   │   └── Timestamp, Node_ID, Temperature_C,
│   │       Humidity_PCT, Pressure_hPa, Flags
│   ├── Check for write errors
│   ├── Sync data to SD card
│   ├── Close file
│   ├── Write or sync failed
│   │   ├── sdStatus = false
│   │   └── return false
│   └── return true
│
├── update()
│   ├── Is sdStatus == true?
│   │   └── Yes → return
│   ├── Check time since last retry
│   │   └── Less than 5 seconds → return
│   ├── Update lastRetryMs
│   ├── Call begin()
│   │   ├── Successful → Print recovery successful
│   │   └── Failed → Print recovery failed
│   └── End
│
├── logSensorData(timestamp, nodeId, T, RH, P, flags)
│   ├── Is sdStatus == false?
│   │   └── Yes → return false
│   ├── Open LOG_FILE using FILE_WRITE
│   │   └── Failed
│   │       ├── sdStatus = false
│   │       ├── lastRetryMs = millis()
│   │       └── return false
│   ├── Write timestamp and nodeId
│   ├── Write temperature
│   │   ├── Flag 0x01 set → Write temperature (2 decimal places)
│   │   └── Otherwise → Write "NA"
│   ├── Write humidity
│   │   ├── Flag 0x02 set → Write humidity (2 decimal places)
│   │   └── Otherwise → Write "NA"
│   ├── Write pressure
│   │   ├── Flag 0x04 set → Write pressure (2 decimal places)
│   │   └── Otherwise → Write "NA"
│   ├── Write sensor flags in hexadecimal format
│   ├── Check for write errors
│   ├── Sync data to SD card
│   ├── Close file
│   ├── Write or sync failed
│   │   ├── sdStatus = false
│   │   ├── lastRetryMs = millis()
│   │   └── return false
│   └── return true
│
└── isReady()
    └── return sdStatus
    ```