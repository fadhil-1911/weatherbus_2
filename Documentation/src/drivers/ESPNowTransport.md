# ESPNowTransport

**Header:** `src/drivers/ESPNowTransport.h`  
**Implementation:** `src/drivers/ESPNowTransport.cpp`

## Purpose

`ESPNowTransport` implements the `ITransport` interface using ESP-NOW for wireless communication between WeatherBus nodes and the Base Station.

It handles transport initialization, peer registration, packet transmission, and reception through the ESP-NOW callback mechanism.

The implementation configures Wi-Fi differently depending on the build target:
- **Base Station (`WEATHERBUS_BASE`)** — uses `WIFI_AP_STA` to support Access Point and Station modes simultaneously.
- **Node** — uses `WIFI_STA` and explicitly sets Wi-Fi channel `6` to match the iPhone hotspot.

## Class Structure

`ESPNowTransport` inherits from `ITransport`.

### Public Functions

| Function | Description |
|---|---|
| `ESPNowTransport()` | Initializes the callback pointer and readiness state, and assigns the instance pointer. |
| `begin(callback)` | Configures Wi-Fi according to the build target, initializes ESP-NOW, reports the current Wi-Fi channel, and registers the receive callback. |
| `addPeer(address)` | Registers an ESP-NOW peer using its MAC address. |
| `send(destination, data, length)` | Sends data to a specified destination. |
| `update()` | Performs no periodic processing because reception is callback-driven. |
| `isReady()` | Returns the transport readiness state. |

### Private Members

| Member | Description |
|---|---|
| `ESP_NOW_ADDRESS_LENGTH` | Defines the required ESP-NOW MAC address length as 6 bytes. |
| `receiveCallback` | Stores the application-level receive callback. |
| `ready` | Indicates whether ESP-NOW initialization succeeded. |
| `instance` | Static pointer used to route the ESP-NOW callback to the class instance. |

### Private Functions

| Function | Description |
|---|---|
| `onDataReceived(mac, data, len)` | Static ESP-NOW receive callback. |
| `handleReceive(data, length)` | Forwards received data to the registered transport callback. |

## Initialization

### `begin(callback)`

The function:

1. Stores the receive callback.
2. Configures Wi-Fi according to the build target:
   - `WEATHERBUS_BASE` defined: sets `WIFI_AP_STA`.
   - Otherwise: sets `WIFI_STA` and configures Wi-Fi channel `6`.
3. For Node builds, returns `false` if setting channel `6` fails.
4. Prints the device MAC address.
5. Initializes ESP-NOW.
6. Returns `false` if ESP-NOW initialization fails.
7. Queries and prints the current Wi-Fi channel. Failure to query the channel is reported but does not stop initialization.
8. Registers the ESP-NOW receive callback.
9. Sets `ready = true` and reports successful initialization.

Returns `true` when initialization succeeds.

## Peer Registration

### `addPeer(address)`

Before registering a peer, the function checks that:

- The transport is ready.
- The address length is exactly 6 bytes.

It then configures the peer using the supplied MAC address, Wi-Fi channel `0`, and encryption disabled.

Returns `true` if `esp_now_add_peer()` succeeds. Otherwise, returns `false`.

## Data Transmission

### `send(destination, data, length)`

The function validates that:

- The transport is ready.
- The destination address length is 6 bytes.
- The data pointer is not null.
- The data length is greater than zero.

It then calls `esp_now_send()` to transmit the data.

Returns `true` when the ESP-NOW API accepts the send request, or `false` if validation or the API call fails.

**A return value of `true` does not confirm that the destination received the packet.**

## Data Reception

### `onDataReceived(mac, data, len)`

The static ESP-NOW callback:

1. Ignores the source MAC address.
2. Checks that the class instance exists.
3. Rejects null data pointers and non-positive lengths.
4. Converts the received length to `size_t`.
5. Forwards the received data to `handleReceive()`.

### `handleReceive(data, length)`

Checks whether an application-level receive callback has been registered.

If available, it forwards the received data and length to that callback.

Packet interpretation and protocol validation are handled by the receiving service or application, not by this transport implementation.

## Update and Status

### `update()`

The function is intentionally empty because ESP-NOW reception is callback-driven and requires no periodic processing in this implementation.

### `isReady()`

Returns the current value of `ready`.

## Dependencies

- `ESPNowTransport.h`
- `ITransport.h`
- `DebugSerial.h`
- Arduino Wi-Fi library (`WiFi.h`)
- ESP-NOW library (`esp_now.h`)
- ESP Wi-Fi library (`esp_wifi.h`)
- `memcpy()`

## Behavioural Notes and Limitations

- ESP-NOW MAC addresses must be exactly 6 bytes long.
- The Base Station uses `WIFI_AP_STA`; Node builds use `WIFI_STA`.
- Node builds explicitly set Wi-Fi channel `6`. Failure to set the channel prevents initialization.
- The current Wi-Fi channel is queried and reported, but query failure does not prevent initialization.
- Peer configuration uses channel `0` and disables encryption.
- The source MAC address of received packets is ignored.
- The transport does not validate WeatherBus packet headers, packet types, sequence numbers, or CRC.
- The transport does not register a send-completion callback.
- Successful transmission API submission does not guarantee packet delivery.
- The implementation uses a static instance pointer, so the receive callback is designed around a single active `ESPNowTransport` instance.
- `ready` indicates successful ESP-NOW initialization; it does not independently verify peer availability or delivery status.
- `DebugSerial.h` is included, but this implementation uses `Serial` for its diagnostic output.

## Test Status

**Code review:** Initialization, build-dependent Wi-Fi configuration, Node channel selection, peer registration, transmission, callback-based reception, and readiness reporting are documented from the provided source.

Hardware communication, channel compatibility, and packet-delivery test results should be recorded separately.

## WeatherBus_2 — `ESPNowTransport.cpp` Flow Tree

```text
ESPNowTransport
│
├── GLOBAL
│   └── instance
│       └── Static pointer to ESPNowTransport object
│
├── Constructor()
│   ├── receiveCallback = nullptr
│   ├── ready = false
│   └── instance = this
│
├── begin(callback)
│   │
│   ├── Store receive callback
│   │
│   ├── Configure Wi-Fi
│   │   │
│   │   ├── WEATHERBUS_BASE defined
│   │   │   └── WiFi.mode(WIFI_AP_STA)
│   │   │
│   │   └── Node build
│   │       ├── WiFi.mode(WIFI_STA)
│   │       ├── Set Wi-Fi channel = 6
│   │       └── Channel setup failed
│   │           └── ready = false → Return false
│   │
│   ├── Print device MAC address
│   │
│   ├── Initialize ESP-NOW
│   │   ├── Failed → ready = false → Return false
│   │   └── Success
│   │       ├── Query current Wi-Fi channel
│   │       │   ├── Success → Print channel
│   │       │   └── Failed → Print error, continue
│   │       ├── Register onDataReceived()
│   │       ├── ready = true
│   │       └── Return true
│
├── addPeer(address)
│   │
│   ├── Check transport ready
│   │   └── Not ready → Return false
│   │
│   ├── Validate address length
│   │   └── Invalid → Return false
│   │
│   ├── Prepare peer information
│   │   ├── Copy peer MAC address
│   │   ├── Channel = 0
│   │   └── Encryption = disabled
│   │
│   ├── Call esp_now_add_peer()
│   │   ├── Failed → Return false
│   │   └── Success → Return true
│
├── send(destination, data, length)
│   │
│   ├── Check transport ready
│   │   └── Not ready → Return false
│   │
│   ├── Validate destination address length
│   │   └── Invalid → Return false
│   │
│   ├── Validate data pointer and length
│   │   └── Invalid → Return false
│   │
│   ├── Call esp_now_send()
│   │   ├── API error → Return false
│   │   └── API accepted → Return true
│
├── update()
│   └── No periodic processing
│       └── ESP-NOW uses callbacks
│
├── isReady()
│   └── Return ready status
│
├── onDataReceived(mac, data, len)
│   │
│   ├── Ignore sender MAC
│   ├── Check instance exists
│   │   └── No instance → Return
│   ├── Validate data pointer and length
│   │   └── Invalid → Return
│   └── Call instance->handleReceive()
│
└── handleReceive(data, length)
    │
    ├── Check receiveCallback
    │   └── nullptr → Return
    │
    └── Invoke receiveCallback(data, length)


```


### DATA SEND FLOW (TX)

```text
    Caller
  │
  ▼
send(destination, data, length)
  │
  ├── Validate transport status
  ├── Validate destination address length
  ├── Validate data pointer and length
  │
  ▼
esp_now_send()
  │
  ├── API error → Return false
  │
  └── API accepted → Return true

```

### DATA RECEIVE FLOW (RX)

```text

  Remote ESP-NOW Device
       │
       ▼
ESP-NOW Receive Callback
       │
       ▼
onDataReceived(mac, data, len)
       │
       ├── Ignore sender MAC
       ├── Check instance
       ├── Check data pointer and length
       │
       ▼
handleReceive(data, length)
       │
       ├── Check receiveCallback
       │
       ▼
receiveCallback(data, length)
       │
       ▼
Caller handles packet
(e.g. Base Station main.cpp)

```

