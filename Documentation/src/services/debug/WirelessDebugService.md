# WirelessDebugService — `WirelessDebugService.h / .cpp`

## 1. Purpose

Provides a wireless debugging service for WeatherBus_2 using an ESP32 Wi-Fi access point and a TCP server.

Allows an external TCP client to connect to the device and receive debug output through `DebugSerial`, while preserving the physical USB Serial output.

**Source:**
- `src/services/debug/WirelessDebugService.h`
- `src/services/debug/WirelessDebugService.cpp`

## 2. Responsibilities

- Configure Wi-Fi in `WIFI_AP_STA` mode.
- Start a Wi-Fi access point for wireless debugging.
- Configure the access point SSID, password and Wi-Fi channel.
- Start a TCP server on port `3333`.
- Accept incoming TCP debug client connections.
- Replace an existing connected client when a new client is accepted.
- Register the active TCP client with `DebugSerial`.
- Detect client disconnection and clear the registered client.
- Call `DebugSerial.update()` to transmit buffered debug output.
- Report access point status and client connection events through `DebugSerial`.

## 3. Module Dependencies

| Module / Component | Responsibility |
|---|---|
| `WirelessDebugService.h` | Declares the service class and its configuration |
| `WirelessDebugService.cpp` | Implements access point, TCP client and update management |
| `WiFi.h` | Provides Wi-Fi access point, server and client APIs |
| `WiFiServer` | Listens for incoming TCP connections |
| `WiFiClient` | Represents the active TCP debug connection |
| `DebugSerial` | Handles USB output and buffered TCP debug transmission |
| Arduino `Serial` | Physical serial output used internally by `DebugSerial` |

## 4. Configuration

| Setting | Value | Purpose |
|---|---|---|
| `TCP_PORT` | `3333` | TCP server listening port |
| `WIFI_CHANNEL` | `1` | Wi-Fi access point channel |
| `apSSID` | `WeatherBus-Debug` | Access point name |
| `apPassword` | `WB2Debug88` | Access point password |
| Wi-Fi mode | `WIFI_AP_STA` | Enables access point and station modes |

The TCP server is constructed using `TCP_PORT`.

The access point password is defined in the source header. Treat it as development configuration and review it before distributing firmware.

## 5. Initialization Sequence

The `begin()` function performs these operations:

1. Set Wi-Fi mode to `WIFI_AP_STA`.
2. Attempt to start the access point using the configured SSID, password and channel.
3. If access point startup fails, print an error through `DebugSerial` and return `false`.
4. Start the TCP server using `server.begin()`.
5. Enable no-delay mode on the server using `server.setNoDelay(true)`.
6. Print the wireless debugging banner.
7. Report the SSID, access point IP address and TCP port.
8. Print a message indicating that the service is waiting for a debug client.
9. Return `true`.

### Initialization Failure Behaviour

| Component | Current Behaviour |
|---|---|
| Wi-Fi mode configuration | Return value is not checked |
| Access point startup | Reports failure and returns `false` |
| TCP server startup | No explicit failure check |
| Debug banner | Printed after successful access point startup |

The caller must decide how to handle a `false` return from `begin()`.

## 6. Client Management — `update()`

The `update()` function manages the TCP connection and services buffered debug output.

### New Client Handling

1. Check `server.hasClient()`.
2. Accept the incoming connection using `server.accept()`.
3. Verify that the returned client is valid.
4. If an existing client is connected, stop that connection.
5. Assign the new client to the service's `client` object.
6. Enable no-delay mode on the new client.
7. Register the client pointer with `DebugSerial.setClient(&client)`.
8. Print a client-connected message.

The service maintains one active client object. When a new client is accepted, an existing connected client is stopped before the new connection is registered.

### Disconnection Handling

After checking for new connections:

1. Check whether the stored client exists but is no longer connected.
2. Stop the client.
3. Clear the client pointer in `DebugSerial` using `DebugSerial.setClient(nullptr)`.

The service then calls `DebugSerial.update()`.

## 7. Main Flow Tree

```text
WirelessDebugService
│
├── CLASS CONFIGURATION
│   ├── TCP_PORT = 3333
│   ├── WIFI_CHANNEL = 1
│   ├── apSSID
│   ├── apPassword
│   ├── WiFiServer server
│   └── WiFiClient client
│
├── begin()
│   ├── WiFi.mode(WIFI_AP_STA)
│   ├── Start Wi-Fi access point
│   │   ├── Failure
│   │   │   ├── Print error
│   │   │   └── Return false
│   │   └── Success
│   │       ├── server.begin()
│   │       ├── server.setNoDelay(true)
│   │       ├── Print debug banner
│   │       ├── Print SSID
│   │       ├── Print access point IP
│   │       ├── Print TCP port
│   │       └── Return true
│
└── update()
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
        ├── Check registered client
        ├── Read buffered debug bytes
        └── Transmit data through TCP
```

## 8. Wireless Debug Data Flow

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

### Connection Lifecycle

```text
WirelessDebugService.begin()
          │
          ▼
Start Wi-Fi Access Point
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
                  └── Continue waiting for connections
```

## 9. Error Handling and Limitations

- If access point startup fails, `begin()` returns `false`.
- TCP server startup is not explicitly verified.
- The Wi-Fi mode configuration result is not checked.
- The service supports one active debug client at a time.
- A newly accepted client replaces the existing connected client.
- The service does not explicitly report errors from TCP server acceptance or transmission.
- `DebugSerial` handles buffer overflow independently; new debug bytes may be dropped when its ring buffer is full.
- Debug output generated while disconnected may remain buffered until the buffer fills or a client reconnects.
- The service does not provide message framing, application-level acknowledgements or authentication beyond the configured Wi-Fi access point password.
- The implementation depends on the application calling `update()` regularly.
- The supplied code does not explicitly handle multiple queued connection attempts beyond the client returned by `server.accept()`.

These points describe the supplied implementation and should not be interpreted as evidence that all failure cases have been tested.

## 10. Development Rules

When modifying `WirelessDebugService.h` or `WirelessDebugService.cpp`:

1. Keep Wi-Fi access point and TCP connection management inside `WirelessDebugService`.
2. Keep debug output buffering and transmission inside `DebugSerial`.
3. Register the active client with `DebugSerial` only while its object remains valid.
4. Clear the registered client when the connection is disconnected or replaced.
5. Preserve USB Serial output independently of TCP connectivity.
6. Keep the TCP port and access point configuration consistent with the documented connection procedure.
7. Ensure `update()` is called regularly from the application loop.
8. Review connection replacement and buffer behaviour before adding multi-client support.
9. Review access point credentials before preparing firmware for wider distribution.
10. Update this document when connection handling or initialization behaviour changes.

## 11. Test Status

The supplied source code establishes the intended implementation, but does not establish that the following behaviours have passed runtime tests:

- Successful access point startup.
- TCP connection using port `3333`.
- Reception of debug output by an external TCP client.
- Replacement of an existing client by a new client.
- Buffer retention while disconnected.
- Recovery after client disconnection and reconnection.
- Behaviour when access point or TCP server initialization fails.

Record verified results in the project's testing documentation against the relevant firmware revision.

