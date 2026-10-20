# ITransport Interface

## 1. Purpose

Defines the abstract communication interface for WeatherBus. It establishes a common contract for initializing a transport, registering peers, sending data, processing transport updates, and checking transport readiness.

Source: `src/interfaces/ITransport.h`

## 2. TransportAddress Structure

`TransportAddress` represents a transport endpoint address.

| Field | Type | Description |
|---|---|---|
| `MAX_LENGTH` | `uint8_t` | Maximum address storage length: 8 bytes |
| `data` | `uint8_t[8]` | Stores the address bytes |
| `length` | `uint8_t` | Number of address bytes used |

The constructor initializes all address bytes to zero and sets `length` to `0`.

The actual address format and required length depend on the transport implementation.

## 3. Receive Callback

```cpp
using TransportReceiveCallback =
    void (*)(const uint8_t* data, size_t length);