# DebugSerial — `DebugSerial.cpp`

## 1. Purpose

Provides a dual-output debug serial service for the WeatherBus system.

Debug messages are written to the physical Arduino Serial interface and copied into a ring buffer for optional TCP transmission over Wi-Fi.

**Source:** `src/services/debug/DebugSerial.cpp`

## 2. Responsibilities

- Initialize the physical Serial interface.
- Store the active Wi-Fi TCP client pointer.
- Forward individual bytes and data buffers to USB Serial.
- Copy written data into the Wi-Fi transmission buffer.
- Protect shared ring-buffer operations using a critical section.
- Drop new bytes when the ring buffer is full.
- Transmit buffered data through the connected TCP client.
- Process TCP transmission through `update()` rather than inside `write()`.
- Retain unsent buffered data when a TCP write reports zero bytes sent.

## 3. Module Dependencies

| Module / Component | Responsibility |
|---|---|
| `DebugSerial.h` | Declares `DebugSerialClass`, its members and buffer configuration |
| `WiFi.h` | Provides `WiFiClient` |
| Arduino `Serial` | Provides physical USB serial output |
| `WiFiClient` | Sends buffered debug data through TCP |
| FreeRTOS critical-section API | Protects ring-buffer index and data operations |
| `DebugSerialClass` | Implements the dual-output debug service |

## 4. Initialization and Client Registration

### `begin(unsigned long baudRate)`

1. Calls `Serial.begin(baudRate)`.
2. Initializes the physical serial interface at the requested baud rate.

### `setClient(WiFiClient* newClient)`

1. Receives a pointer to a `WiFiClient`.
2. Stores the pointer in `client`.
3. Does not establish a TCP connection or verify client connectivity.

The TCP client connection must be established and maintained by the surrounding application.

## 5. Write Operations

### `write(uint8_t data)`

1. Writes one byte to physical Serial.
2. Copies the byte into the Wi-Fi ring buffer using `enqueue()`.
3. Returns `1`.

### `write(const uint8_t* data, size_t size)`

1. Rejects a null pointer or zero-length buffer by returning `0`.
2. Writes the complete supplied buffer to physical Serial.
3. Copies each byte into the Wi-Fi ring buffer.
4. Returns the requested `size`.

The return value represents the requested byte count, not the number of bytes successfully queued or transmitted over TCP.

USB Serial output is independent of Wi-Fi connectivity. Buffer overflow affects the Wi-Fi copy, not the preceding USB write.

## 6. Ring Buffer Management

### `enqueue(uint8_t data)`

1. Enters a critical section using `mux`.
2. Calculates the next head index.
3. Checks whether the next index would collide with `tail`.
4. Stores the byte and advances `head` when space is available.
5. Drops the new byte when the buffer is full.
6. Exits the critical section.

The buffer uses a ring-buffer design with `head` and `tail` indices.

In a conventional ring buffer that reserves one slot to distinguish full from empty, usable capacity is `BUFFER_SIZE - 1`. Confirm the actual declaration in `DebugSerial.h`.

The operation does not wait for TCP connectivity or buffer space.

## 7. TCP Transmission — `update()`

The `update()` function performs the following operations:

1. Returns immediately if `client` is null or disconnected.
2. Creates a local transmission chunk of 256 bytes.
3. Enters a critical section and calculates the buffered byte count.
4. Limits the chunk to a maximum of 256 bytes.
5. Copies the selected bytes into the local chunk.
6. Exits the critical section.
7. Returns if no bytes are available.
8. Calls `client->write(chunk, count)`.
9. Advances `tail` by the number of bytes reported as sent, if that number is greater than zero.

TCP transmission occurs outside the critical section, avoiding holding the buffer lock during the network write.

The function must be called periodically by the application, typically from `loop()`.

## 8. Main Flow Tree

```text
DebugSerial.cpp
│
├── GLOBAL OBJECT
│   └── DebugSerial
│       └── DebugSerialClass
│
├── begin(baudRate)
│   └── Serial.begin(baudRate)
│
├── setClient(newClient)
│   └── Store WiFiClient pointer
│
├── write(data)
│   ├── Serial.write(data)
│   └── enqueue(data)
│
├── write(data, size)
│   ├── Validate pointer and size
│   ├── Serial.write(data, size)
│   └── FOR EACH BYTE
│       └── enqueue(data[i])
│
├── enqueue(data)
│   ├── Enter critical section
│   ├── Calculate next head index
│   ├── Check buffer capacity
│   ├── Store byte and advance head
│   ├── OR drop byte if full
│   └── Exit critical section
│
└── update()
    ├── Check client pointer and connection
    ├── Copy up to 256 buffered bytes
    ├── Exit critical section
    ├── Send chunk through TCP
    └── Advance tail by bytes sent
```


## 9. Debug Data Flow

```text
Debug Message
     │
     ▼
DebugSerial.write()
     │
     ├──► Physical Serial
     │      └── USB debug output
     │
     └──► enqueue()
              │
              ├── Buffer has space
              │      └── Store byte
              │
              └── Buffer full
                     └── Drop new byte

TCP Transmission
     │
     ▼
loop()
     │
     ▼
DebugSerial.update()
     │
     ├── Client unavailable
     │      └── Return; retain buffer
     │
     ├── Copy up to 256 bytes
     │
     └── client.write()
              │
              ├── Bytes sent > 0
              │      └── Advance tail
              │
              └── Bytes sent = 0
                     └── Retain buffered data
```

## 10. Error Handling and Limitations

- USB Serial output does not depend on an active TCP client.
- A null or disconnected TCP client causes `update()` to return without draining the buffer.
- A full ring buffer drops newly enqueued bytes without reporting the loss to the caller.
- A null data pointer or zero-length buffer causes the buffer overload of `write()` to return `0`.
- The `write()` methods do not report Wi-Fi queueing failures or TCP transmission failures.
- TCP transmission is limited to 256 bytes per `update()` call.
- The code does not explicitly handle a negative or otherwise invalid client write result; the return type of `WiFiClient::write()` is unsigned `size_t`.
- The code does not explicitly validate `newClient` before storing it, although `update()` checks whether `client` is null.
- The lifetime of the `WiFiClient` object must exceed its use through the stored pointer.
- The implementation does not provide delivery acknowledgements at the application level.

## 11. Development Rules

When modifying `DebugSerial.cpp`:

1. Preserve physical Serial output independently of Wi-Fi transmission.
2. Do not perform TCP writes inside `write()` or `enqueue()`.
3. Keep ring-buffer index and data access protected by the appropriate critical section.
4. Advance `tail` only by the number of bytes reported as sent.
5. Preserve buffered data when the client is disconnected or sends zero bytes.
6. Keep buffer sizing and index definitions consistent with `DebugSerial.h`.
7. Ensure the TCP client remains valid for as long as `DebugSerial` uses its pointer.
8. Call `DebugSerial.update()` regularly from the application loop.
9. Review buffer overflow behaviour if debug output volume increases.
10. Update this documentation when the buffer design, transmission behaviour or error handling changes.

## 12. Test Status

The supplied source code establishes the implementation structure, but does not establish that the following behaviours have passed runtime tests:

- Physical Serial output while Wi-Fi is disconnected.
- TCP output after a client connects.
- Buffer overflow and dropped-byte behaviour.
- Partial TCP writes and subsequent transmission.
- Recovery after a client disconnects and reconnects.
- Correct operation under concurrent buffer access.

Record test results separately when these behaviours have been verified against the current firmware.

