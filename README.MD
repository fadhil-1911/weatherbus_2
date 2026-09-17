# WeatherBus

**Version:** 1.0
**Last Updated:** 2026-09-14

---

**Project:** WeatherBus V1.0

**Transport:** ESP-NOW

**Current Phase:** **PHASE 2C.7 - Regression + Stability**

---

## Updates

* Added ESP-NOW initialization and peer configuration.
* Added DATA_REQUEST reception and validation.
* Added SENSOR_DATA response handling.
* Added WeatherBus protocol header validation.
* Replaced dummy temperature and humidity data with real SHT41 sensor readings.
* Added SHT41 I²C communication using GPIO 8 (SDA) and GPIO 9 (SCL).
* Added SHT41 communication detection during node startup.
* Added SHT41 measurement and data conversion.
* Added SHT41 CRC validation for temperature and humidity data.
* Added request sequence synchronization between Base and Node.
* Added multi-node sequential polling support.
* Added 200 ms response timeout handling at Base.
* Verified node offline timeout and automatic recovery.
* Verified continuous polling with two sensor nodes.
* Added invalid packet size validation at Node.
* Added protocol version validation at Node.
* Added packet type validation at Node.
* Added Node ID validation at Node.
* Added payload length validation for DATA_REQUEST.
* Invalid packets are rejected without generating a SENSOR_DATA response.
* Added Node ID validation at Base.
* Added current-node validation at Base.
* Added SENSOR_DATA sequence validation at Base.
* Responses with unexpected sequence numbers are rejected.
* Added sensor temperature validation at Base.
* Added sensor humidity validation at Base.
* Added non-finite floating-point value validation using `isfinite()`.
* Invalid sensor values are rejected without being accepted as valid SENSOR_DATA.
* Added Base response-state validation.
* SENSOR_DATA received when no request is active is rejected.
* Added late response handling after request timeout.
* Verified that delayed SENSOR_DATA does not corrupt the current polling cycle.
* Verified Node failure, timeout, recovery, and continued sequential polling.
* Verified Node recovery without restarting the Base.
* Verified repeated normal operation after Node reconnection.
* Added BME280 sensor support.
* Added BME280 I²C communication using address `0x76`.
* Added BME280 pressure measurement in hPa.
* Extended `SensorDataPayload` to include pressure.
* Extended WeatherBus sensor flags to identify valid pressure data.
* Added conditional BME280 support in the generic Node firmware.
* Node 1 operates with SHT41 only.
* Node 2 operates with SHT41 + BME280.
* Added non-fatal BME280 startup failure handling.
* Added runtime BME280 failure handling.
* Verified that SHT41 temperature and humidity continue operating when BME280 is unavailable.
* Added BME280 recovery attempt after invalid pressure readings.
* Verified BME280 recovery after I²C interruption.
* Identified a BME280 false-valid pressure edge case where approximately `673.62 hPa` can persist after certain disconnect/reconnect conditions.
* Confirmed that resetting the affected Node restores normal BME280 pressure readings.
* Classified the `673.62 hPa` condition as a known edge case for future investigation rather than modifying the current firmware immediately.

---

## Hardware Test

Current Phase 2C hardware configuration:

* 1 × WeatherBus Base
* 2 × WeatherBus Nodes
* 2 × SHT41 sensors
* 1 × BME280 sensor
* ESP32-C3
* ESP-NOW wireless transport

### SHT41 Wiring

| SHT41 | ESP32-C3 |
| ----- | -------- |
| SDA   | GPIO 8   |
| SCL   | GPIO 9   |

### BME280 Wiring

BME280 shares the same I²C bus as SHT41.

| BME280      | ESP32-C3 |
| ----------- | -------- |
| SDA         | GPIO 8   |
| SCL         | GPIO 9   |
| I²C Address | `0x76`   |

### Node Configuration

Each node uses the same generic firmware.

Node capabilities are configured at compile time:

```cpp
constexpr uint8_t NODE_ID = 1;

constexpr bool ENABLE_SHT41 = true;
constexpr bool ENABLE_BME280 = false;
```

For Node 2:

```cpp
constexpr uint8_t NODE_ID = 2;

constexpr bool ENABLE_SHT41 = true;
constexpr bool ENABLE_BME280 = true;
```

The Base does not need prior knowledge of the physical sensor configuration of each Node.

Sensor capability is indicated through the WeatherBus sensor flags.

---

# Phase 2A Test Status

**PASS**

Previously verified:

* Base → Node DATA_REQUEST.
* Node → Base SENSOR_DATA.
* Node 1 and Node 2 sequential polling.
* Temperature and humidity from real SHT41 sensors.
* Sequence synchronization.
* Response time below 200 ms.
* Node timeout handling.
* Continued operation when a node is offline.
* Automatic recovery when the node returns online.

---

# Phase 2B Test Status

## Phase 2B.1

**PASS**

Verified:

* Invalid packet size handling.
* Protocol version validation.
* Packet type validation.
* Node ID validation.
* Payload length validation.
* Invalid packets are rejected without generating SENSOR_DATA responses.

---

## Phase 2B.2

**PASS**

Verified:

* Node response timeout.
* Node offline detection.
* Continued polling of available nodes.
* Automatic node recovery after reconnection.
* Repeated node disconnect/reconnect operation.

The Base continues polling other nodes when one Node becomes unavailable.

---

## Phase 2B.3

**PASS**

Verified:

* SHT41 communication failure detection.
* `SENSOR_READ_FAILED` handling.
* No fake sensor data is transmitted when SHT41 read fails.
* Node firmware continues running during sensor failure.
* SHT41 operation recovers after sensor reconnection.

---

## Phase 2B.4

**PASS — LOCKED**

Verified:

* Base stores expected sequence for each DATA_REQUEST.
* Node echoes request sequence in SENSOR_DATA.
* Base rejects unexpected sequence numbers.
* Sequence mismatch does not generate valid WEATHERBUS RX.
* Timeout occurs when no valid matching response is received within 200 ms.
* Normal synchronization resumes after test condition is removed.

---

## Phase 2B.5

**PASS — LOCKED**

### Sensor Data Validation

Temperature:

```text
-40.0 °C ≤ Temperature ≤ 85.0 °C
```

Humidity:

```text
0.0 % ≤ Humidity ≤ 100.0 %
```

Non-finite values are rejected:

```text
NaN
+Infinity
-Infinity
```

Validation order:

```text
Packet size
    ↓
Protocol version
    ↓
Packet type
    ↓
Payload length
    ↓
Node ID
    ↓
Current Node
    ↓
Sequence
    ↓
Temperature
    ↓
Humidity
    ↓
Accept SENSOR_DATA
    ↓
Mark Node online
```

Negative testing using an intentionally invalid temperature of `150.00 °C` was rejected correctly.

**WeatherBus V1.0 Phase 2B.5 is functionally validated and LOCKED.**

---

## Phase 2B.6

**PASS — LOCKED**

### Late Response Handling

`SENSOR_DATA` is processed only when the polling engine is actively waiting for a response.

```text
State != WAIT_RESPONSE
        ↓
Reject response
```

A controlled 300 ms Node 2 delay was tested against the 200 ms Base timeout.

Observed:

```text
TX: DATA_REQUEST -> Node 2
TIMEOUT: Node 2 did not respond within 200 ms
RX: Ignored response | No active request
```

The delayed response did not corrupt the polling cycle.

**WeatherBus V1.0 Phase 2B.6 is functionally validated and LOCKED.**

---

## Phase 2B.7

**PASS — LOCKED**

### Node Recovery & Repeated Failure

Verified:

1. Node 1 + Node 2 normal.
2. Node 2 operates normally.
3. Node 2 power is disconnected.
4. Base detects Node 2 timeout.
5. Node 1 continues operating.
6. Node 2 is powered on again.
7. Node 2 responds normally.
8. Sequential polling returns to normal.

Verified:

* Node 2 timeout detection.
* Node 1 continued operating during Node 2 failure.
* Node 2 successfully recovered.
* Node 2 returned valid SHT41 data after recovery.
* Sequence numbers remained synchronized.
* No sequence mismatch occurred.
* No unexpected timeout occurred after recovery.
* Base did not require restart.
* Polling engine did not become stuck.

**WeatherBus V1.0 Phase 2B.7 is functionally validated and LOCKED.**

---

# Phase 2C — BME280 Integration

## Phase 2C.1 — BME280 I²C Detection

**PASS — LOCKED**

BME280 was detected on:

```text
0x76
```

SHT41 remained detected on:

```text
0x44
```

Both sensors operate on the same I²C bus.

---

## Phase 2C.2 — Raw Pressure Reading

**PASS — LOCKED**

Raw BME280 pressure register data was successfully read and observed to change slightly during normal operation.

---

## Phase 2C.3 — Pressure → hPa

**PASS — LOCKED**

BME280 pressure conversion was validated using the Adafruit BME280 library.

Normal observed pressure:

```text
≈ 1009–1013 hPa
```

Typical test readings:

```text
Temperature : ≈ 32.9 °C
Pressure    : ≈ 1009.4 hPa
Humidity    : ≈ 63.3 %
```

---

## Phase 2C.4 — Sensor Data / WBP Extension

**PASS — LOCKED**

`SensorDataPayload` was extended:

```cpp
struct SensorDataPayload {
    float temperature;
    float humidity;
    float pressure;
};
```

Packet size:

```text
Header              12 bytes
SensorDataPayload   12 bytes
Total               24 bytes
```

Pressure validity flag:

```cpp
FLAG_PRESSURE_VALID = 0x04;
```

Examples:

```text
Flags = 0x03
Temperature + Humidity valid

Flags = 0x07
Temperature + Humidity + Pressure valid
```

---

## Phase 2C.5 — Base / Node Integration

**PASS — LOCKED**

Verified:

### Node 1

```text
SHT41
 ↓
Temperature
Humidity
 ↓
Flags = 0x03
```

### Node 2

```text
SHT41 + BME280
 ↓
Temperature
Humidity
Pressure
 ↓
Flags = 0x07
```

Example:

```text
Node=2
Flags=0x07
T≈32.1 °C
RH≈68.3 %
P≈1013 hPa
```

Response time remained well below the 200 ms timeout.

The Base correctly displays pressure only when `FLAG_PRESSURE_VALID` is set.

---

# Phase 2C.6 — Failure Handling

**CONDITIONAL PASS — KNOWN EDGE CASE**

### BME280 Missing at Startup

Verified:

```text
SHT41 communication OK
BME280 communication failed
BME280 unavailable - continuing without pressure
```

Node continues operating.

---

### BME280 Missing During Runtime

Verified:

```text
I²C Error
    ↓
BME280 pressure invalid
    ↓
Recovery attempt failed
    ↓
BME280 pressure unavailable
    ↓
Flags = 0x03
```

Temperature and humidity from SHT41 continue operating.

The Node does not stop because of BME280 failure.

---

### BME280 Recovery

BME280 recovery has been successfully observed after I²C interruption.

In successful recovery tests, normal pressure returned:

```text
≈ 1012 hPa
```

---

### Known Edge Case — False-Valid Pressure

A specific disconnect/reconnect condition produced:

```text
Pressure = 673.62 hPa
Flags    = 0x07
```

The value persisted across many polling cycles.

The condition was observed while:

* Temperature continued changing.
* Humidity continued changing.
* ESP-NOW communication remained operational.
* Node continued responding normally.

A Node reset restored normal BME280 pressure readings.

### Current Decision

The `673.62 hPa` condition is classified as:

```text
KNOWN EDGE CASE
```

It is **not being treated as a current blocking issue** for Phase 2C.

No additional pressure continuity algorithm is being added at this stage.

Future firmware revisions, particularly production firmware, may investigate:

* More robust BME280 recovery.
* Measurement-state recovery.
* Pressure plausibility validation.
* Automatic sensor reinitialization.

For the current development firmware, an abnormal pressure reading can be treated as a diagnostic indication and the Node can be reset if required.

Therefore:

**Phase 2C.6 is conditionally validated, with a known BME280 recovery edge case.**

---

# Current Architecture

```text
                         WeatherBus Base
                            ESP32-C3
                                │
                         DATA_REQUEST
                                │
                  ┌─────────────┴─────────────┐
                  │                           │
               Node 1                      Node 2
              ESP32-C3                    ESP32-C3
                  │                           │
                SHT41                  SHT41 + BME280
                  │                           │
             SENSOR_DATA                  SENSOR_DATA
                  │                           │
                  └─────────────┬─────────────┘
                                │
                               Base
```

The Base polls Nodes sequentially.

Each Node responds only when a valid DATA_REQUEST addressed to that Node is received.

The Node performs packet validation before accessing sensors or generating a response.

The Base validates received SENSOR_DATA before accepting it as valid data.

The Base validates:

```text
Packet
  ↓
Node
  ↓
Current Node
  ↓
Sequence
  ↓
Temperature
  ↓
Humidity
  ↓
Pressure (if FLAG_PRESSURE_VALID)
  ↓
Valid SENSOR_DATA
```

The Base also verifies that a response is only processed while the polling engine is in:

```text
WAIT_RESPONSE
```

Responses received outside an active request are ignored.

---

# Current Phase Status

```text
Phase 1       PASS

Phase 2A      PASS

Phase 2B.1    PASS
Phase 2B.2    PASS
Phase 2B.3    PASS
Phase 2B.4    PASS 🔒
Phase 2B.5    PASS 🔒
Phase 2B.6    PASS 🔒
Phase 2B.7    PASS 🔒

Phase 2C.1    PASS 🔒
Phase 2C.2    PASS 🔒
Phase 2C.3    PASS 🔒
Phase 2C.4    PASS 🔒
Phase 2C.5    PASS 🔒
Phase 2C.6    CONDITIONAL PASS
Phase 2C.7    NEXT
```

Current hardware:

```text
1 × Base
2 × Nodes
2 × SHT41
1 × BME280
ESP32-C3
ESP-NOW
```

The current hardware test is limited to two Nodes.

The software architecture uses logical Node IDs and can be extended when additional hardware becomes available.

---

# Next Phase

**PHASE 2C.7 — REGRESSION + STABILITY**

The next phase will verify that the complete BME280-integrated WeatherBus system remains stable during normal operation and after previously validated failure conditions.

Focus:

* Continuous Base ↔ Node communication.
* Node 1 normal operation.
* Node 2 SHT41 + BME280 normal operation.
* Sequential polling.
* Sequence synchronization.
* Response time.
* BME280 pressure stability.
* SHT41 temperature/humidity stability.
* Node offline/recovery regression.
* BME280 failure/recovery regression.
* No Base restart.
* No polling engine lock-up.
* No unexpected packet validation failures.
* Extended continuous operation.

The known `673.62 hPa` BME280 edge case remains documented and is not currently treated as a blocking issue.

---

# Phase 2C Objective

Phase 2C integrates pressure sensing into WeatherBus V1.0 while maintaining the existing communication reliability architecture.

Current protection layers:

```text
Invalid Packet

      ↓

Node Failure

      ↓

Sensor Failure

      ↓

Sequence Validation

      ↓

Sensor Data Validation

      ↓

Response State Validation

      ↓

Node Recovery

      ↓

BME280 Failure Handling

      ↓

Pressure Capability Flag
```

Each layer is independently tested before moving to the next phase.

**Phase 2C.7 is now the next engineering milestone.**
