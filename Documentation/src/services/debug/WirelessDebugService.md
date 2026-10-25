# WirelessDebugService — `WirelessDebugService.h / .cpp`

## 1. Purpose

Provides a wireless debugging service for WeatherBus_2 using Wi-Fi and a TCP server.

Allows an external TCP client connected to the same network to receive debug output through `DebugSerial`, while preserving physical USB Serial output.

The current implementation defaults to **Station (STA) mode**, connecting to an existing Wi-Fi network. Access Point (AP) mode remains available through a compile-time configuration constant.

**Source:**
- `src/services/debug/WirelessDebugService.h`
- `src/services/debug/WirelessDebugService.cpp`

## 2. Responsibilities

- Select between Wi-Fi Station and Access Point modes.
- Connect to an existing Wi-Fi network in STA mode.
- Optionally start a Wi-Fi access point for debugging.
- Start a TCP server on port `3333`.
- Apply a connection timeout of 15 seconds in STA mode.
- Accept incoming TCP debug client connections.
- Replace an existing connected client when a new client is accepted.
- Register the active TCP client with `DebugSerial`.
- Detect client disconnection and clear the registered client.
- Call `DebugSerial.update()` to transmit buffered debug output.
- Report Wi-Fi status, IP address, TCP port and client connection events through `DebugSerial`.

## 3. Module Dependencies

| Module / Component | Responsibility |
|---|---|
| `WirelessDebugService.h` | Declares the service class and its configuration |
| `WirelessDebugService.cpp` | Implements Wi-Fi connection, TCP client and update management |
| `WiFi.h` | Provides Wi-Fi, server and client APIs |
| `WiFiServer` | Listens for incoming TCP connections |
| `WiFiClient` | Represents the active TCP debug connection |
| `DebugSerial` | Handles USB output and buffered TCP debug transmission |
| Arduino `Serial` | Physical serial output used internally by `DebugSerial` |
| `millis()` and `delay()` | Manage the STA connection timeout and retry interval |

## 4. Configuration

### Header Configuration

| Setting | Value | Purpose |
|---|---|---|
| `TCP_PORT` | `3333` | TCP server listening port |
| `WIFI_CHANNEL` | `1` | Wi-Fi channel used in AP mode |
| Class member `apSSID` | `WeatherBus-Debug` | AP SSID declared in the header |
| Class member `apPassword` | Configured in header | AP password declared in the header |
| `WiFiServer server` | `server{TCP_PORT}` | TCP server instance |
| `WiFiClient client` | Default constructed | Stores the active TCP client |

### Implementation Configuration

| Setting | Current Value / Behaviour | Purpose |
|---|---|---|
| `USE_AP_MODE` | `false` | Selects STA mode by default |
| Global `apSSID` | `WeatherBus-Debug` | SSID used by the AP implementation |
| Global `apPassword` | Configured in `.cpp` | Password used by the AP implementation |
| `staSSID` | Configured in `.cpp` | Existing Wi-Fi network name |
| `staPassword` | Configured in `.cpp` | Existing Wi-Fi network password |
| `CONNECT_TIMEOUT_MS` | `15000` ms | Maximum STA connection wait |
| `TCP_PORT` | `3333` | TCP debug port |

**Configuration note:** The AP password defined in the header differs from the global AP password in the `.cpp`. The current implementation uses the global variables in the `.cpp` when starting AP mode; the class members are not used by that code.

The Wi-Fi credentials are hard-coded in the implementation and should be reviewed before distributing firmware.

## 5. Initialization Sequence

### `begin()`

The function selects the Wi-Fi mode using `USE_AP_MODE`.

### AP Mode — `USE_AP_MODE = true`

1. Sets Wi-Fi mode to `WIFI_AP_STA`.
2. Attempts to start the access point using the configured SSID, password and channel.
3. If AP startup fails, prints an error through `DebugSerial` and returns `false`.
4. Prints the selected mode, SSID and AP IP address.
5. Starts the TCP server using `server.begin()`.
6. Enables no-delay mode on the server using `server.setNoDelay(true)`.
7. Prints the TCP debug port.
8. Returns `true`.

### STA Mode — `USE_AP_MODE = false`

1. Sets Wi-Fi mode to `WIFI_STA`.
2. Starts connecting to the configured Wi-Fi network using `WiFi.begin()`.
3. Prints the selected mode and network SSID.
4. Waits for the connection, checking `WiFi.status()` every 250 ms.
5. Stops waiting after `15000` ms if the connection has not succeeded.
6. If connection fails, prints an error and returns `false`.
7. If connected, prints the local IP address.
8. Starts the TCP server using `server.begin()`.
9. Enables no-delay mode on the server using `server.setNoDelay(true)`.
10. Prints the TCP debug port.
11. Returns `true`.

### Initialization Failure Behaviour

| Component | Current Behaviour |
|---|---|
| Wi-Fi mode configuration | Return value is not checked |
| AP startup | Reports failure and returns `false` |
| STA connection | Waits up to 15 seconds; returns `false` on timeout |
| TCP server startup | No explicit failure check |
| TCP no-delay configuration | Enabled; return value is not checked |

The caller must decide how to handle a `false` return from `begin()`.

## 6. Client Management — `update()`

The `update()` function manages the TCP connection and services buffered debug output.

### New Client Handling

1. Checks `server.hasClient()`.
2. Accepts the incoming connection using `server.accept()`.
3. Verifies that the returned client is valid.
4. If an existing client is connected, stops that connection.
5. Assigns the new client to the service's `client` object.
6. Enables no-delay mode on the new client using `client.setNoDelay(true)`.
7. Registers the client pointer with `DebugSerial.setClient(&client)`.
8. Prints a client-connected message.

The service maintains one active client object. A newly accepted client replaces an existing connected client.

### Disconnection Handling

After checking for new connections:

1. Checks whether the stored client exists but is no longer connected.
2. Stops the client.
3. Clears the registered client pointer using `DebugSerial.setClient(nullptr)`.

The service then calls `DebugSerial.update()` to transmit buffered debug output when a client is available.

##  7. Main Flow Tree

```text
WirelessDebugService
│
├── CONFIGURATION
│   ├── USE_AP_MODE = false
│   ├── TCP_PORT = 3333
│   ├── WIFI_CHANNEL = 1
│   ├── AP credentials
│   ├── STA credentials
│   ├── WiFiServer server
│   └── WiFiClient client
│
├── begin()
│   │
│   ├── USE_AP_MODE = true
│   │   ├── WiFi.mode(WIFI_AP_STA)
│   │   ├── Start Wi-Fi access point
│   │   ├── Failure → Print error → Return false
│   │   └── Success
│   │       ├── Print AP SSID and IP
│   │       ├── Start TCP server
│   │       ├── Enable server no-delay
│   │       └── Return true
│   │
│   └── USE_AP_MODE = false
│       ├── WiFi.mode(WIFI_STA)
│       ├── WiFi.begin(staSSID, staPassword)
│       ├── Wait for connection (maximum 15 seconds)
│       ├── Timeout → Print error → Return false
│       └── Connected
│           ├── Print local IP
│           ├── Start TCP server
│           ├── Enable server no-delay
│           └── Return true
│
└── update()
    │
    ├── Check server.hasClient()
    ├── Accept new client
    │   ├── Validate new client
    │   ├── Stop previous connected client
    │   ├── Assign new client
    │   ├── client.setNoDelay(true)
    │   ├── DebugSerial.setClient(&client)
    │   └── Print connection message
    │
    ├── Check client disconnection
    │   ├── client.stop()
    │   └── DebugSerial.setClient(nullptr)
    │
    └── DebugSerial.update()
        └── Transmit buffered debug output through TCP

```

##  8. Wireless Debug Data Flow

```text

WeatherBus_2 Application
          │
          ▼
     DebugSerial
          │
          ├──► USB Serial
          │
          └──► Wi-Fi Ring Buffer
                       │
                       ▼
             WirelessDebugService
                       │
                       ├── Accept TCP client
                       ├── Register client pointer
                       └── DebugSerial.update()
                                  │
                                  ▼
                             WiFiClient
                                  │
                                  ▼
                            TCP Port 3333
                                  │
                                  ▼
                         External Debug Client

```

## 9. Connection Lifecycle

```text
WirelessDebugService.begin()
          │
          ▼
Select Wi-Fi Mode
          │
          ├── AP Mode
          │     └── Start Access Point
          │
          └── STA Mode
                └── Connect to Existing Wi-Fi
                          │
                          ▼
                   Start TCP Server
                          │
                          ▼
                   Wait for TCP Client
                          │
                          ▼
                     Accept Client
                          │
                          ▼
                Register with DebugSerial
                          │
                          ▼
             Transmit Buffered Debug Output
                          │
                          ├── Client remains connected
                          │       └── Continue update()
                          │
                          └── Client disconnects
                                  ├── Stop client
                                  ├── Clear DebugSerial client
                                  └── Continue waiting
```