# PollingEngine

**Source:** `src/base/polling_engine.cpp`

## Purpose

`PollingEngine` manages the Base Station's polling cycle for configured sensor nodes. It sends `DATA_REQUEST` packets, waits for responses, validates received sensor data, and advances to the next node.

## Responsibilities

- Schedule polling cycles using `POLL_INTERVAL_MS`.
- Send `DATA_REQUEST` packets through the `ITransport` interface.
- Track request sequence numbers and response timeouts.
- Process received responses in the main loop.
- Validate node ID, sequence number, and flagged sensor measurements.
- Update node online/offline status.
- Notify the application when a valid response is received.

## State Machine

The polling process uses three states:

| State | Behaviour |
|---|---|
| `IDLE` | Waits until `POLL_INTERVAL_MS` has elapsed. |
| `SEND_REQUEST` | Sends a request to the current node. |
| `WAIT_RESPONSE` | Waits for a response until `RESPONSE_TIMEOUT_MS` expires. |

After a valid response or timeout, the engine advances to the next configured node and returns to `IDLE`.

## Main Functions

### `begin()`

Initializes the polling state, records the current time, clears the pending-response flag, and prints a readiness message.

### `update()`

Runs the polling state machine and processes any pending response.

State transitions are handled in `update()`, rather than directly in the transport callback.

### `sendRequest()`

Builds a `WeatherBus::Header` with:

- Protocol version from `PROTOCOL_VERSION`.
- Packet type `DATA_REQUEST`.
- The current node ID.
- An incremented sequence number.
- Zero payload length.
- CRC set to zero.
- Reserved field set to zero.

The request is sent to the node's configured MAC address through `ITransport`.

If sending fails, the engine handles it as a timeout. Otherwise, it records the request timestamp and enters `WAIT_RESPONSE`.

### `onSensorData()`

Stores the first pending sensor response in a shared buffer protected by `responseMux`.

The transport callback does not directly change the polling state machine. The response is processed later by `update()`.

If a response is already pending, subsequent responses are ignored until the pending response is processed.

### `processPendingResponse()`

Copies the pending response into local variables under a critical section, then validates it.

Validation includes:

1. A response must be expected in the current state.
2. The node ID must be configured.
3. The response must belong to the currently polled node.
4. The sequence number must match the expected sequence.
5. Flagged temperature must be finite and within -40 to 85 °C.
6. Flagged humidity must be finite and within 0 to 100%.
7. Flagged pressure must be finite and within 300 to 1100 hPa.

If validation succeeds, the engine:

- Marks the node online.
- Calculates response time.
- Prints the received sensor data.
- Invokes `validResponseCallback`, if configured.
- Advances to the next polling cycle.

### `handleTimeout()`

Marks the current node offline and prints a timeout message when the node exists, then advances to the next polling cycle.

This function is also used when `transport.send()` fails.

### `moveToNextNode()`

Advances the node index, wraps to index zero when necessary, records the current time, and returns the state machine to `IDLE`.

### `setValidResponseCallback()`

Registers the callback that receives valid sensor responses.

## Dependencies

- `polling_engine.h`
- `NodeManager`
- `ITransport`
- `WeatherBus` protocol definitions
- Arduino `millis()` and `Serial`
- `isfinite()` for measurement validation
- FreeRTOS critical-section primitives for pending-response access

## Behavioural Notes and Limitations

- CRC is disabled for requests and set to zero.
- Only the first response received while the pending buffer is occupied is retained.
- The pending response buffer stores no reception timestamp. A response processed after a delay may therefore be evaluated against the current polling state.
- A response rejected during validation does not immediately advance the polling cycle; the engine remains in `WAIT_RESPONSE` until another response is processed or the timeout expires.
- Measurement validation is performed only when the corresponding validity flag is set.
- The implementation does not explicitly reject unknown flag bits or require at least one validity flag.
- The transport abstraction allows the polling engine to operate independently of a specific communication technology.

## Test Status

**Code review:** Polling states, request handling, response validation, timeout handling, and node progression are documented from the implementation.

Hardware and long-duration test results must be recorded separately.



TREE FLOW

PollingEngine
├── Constructor(nodeManager, transport)
│   ├── Store NodeManager reference
│   ├── Store ITransport reference
│   ├── Set state = IDLE
│   ├── Set currentNodeIndex = 0
│   ├── Initialize sequenceNumber and expectedSequence = 0
│   ├── Initialize timestamps = 0
│   ├── Set responsePending = false
│   └── Initialize pending response data
│
├── begin()
│   ├── Set state = IDLE
│   ├── Set stateTimestamp = millis()
│   ├── Set responsePending = false
│   └── Print "Polling Engine ready"
│
├── update()
│   ├── Get current time
│   ├── Call processPendingResponse()
│   ├── Refresh current time
│   └── Process state machine
│       ├── IDLE
│       │   ├── Check POLL_INTERVAL_MS elapsed
│       │   │   ├── Yes → Set state = SEND_REQUEST
│       │   │   └── No → Remain IDLE
│       │
│       ├── SEND_REQUEST
│       │   └── Call sendRequest()
│       │
│       └── WAIT_RESPONSE
│           ├── Check RESPONSE_TIMEOUT_MS elapsed
│           │   ├── Yes → Call handleTimeout()
│           │   └── No → Continue waiting
│
├── sendRequest()
│   ├── Get node using currentNodeIndex
│   ├── Node unavailable?
│   │   └── Yes → Call moveToNextNode() → return
│   ├── Build DATA_REQUEST header
│   │   ├── Set protocolVersion
│   │   ├── Set packetType = DATA_REQUEST
│   │   ├── Set nodeId
│   │   ├── Set flags = 0
│   │   ├── Increment sequenceNumber
│   │   ├── Store expectedSequence
│   │   ├── Set payloadLength = 0
│   │   ├── Set crc16 = 0
│   │   └── Set reserved = 0
│   ├── Build destination address from node MAC
│   ├── Send request through transport
│   │   ├── Failed
│   │   │   └── Call handleTimeout()
│   │   └── Successful
│   │       ├── Set requestTimestamp = millis()
│   │       ├── Set state = WAIT_RESPONSE
│   │       └── Print transmission details
│
├── onSensorData(nodeId, sequence, flags, T, RH, P)
│   ├── Enter critical section (responseMux)
│   ├── Is responsePending == false?
│   │   ├── Yes
│   │   │   ├── Copy response fields into pending buffer
│   │   │   └── Set responsePending = true
│   │   └── No → Keep existing pending response
│   └── Exit critical section
│
├── processPendingResponse()
│   ├── Enter critical section (responseMux)
│   ├── Is responsePending == false?
│   │   └── Yes → Exit critical section → return
│   ├── Copy pending response into local variables
│   ├── Set responsePending = false
│   └── Exit critical section
│
│   ├── Is state != WAIT_RESPONSE?
│   │   └── Yes → Ignore response → return
│   ├── Find node using received nodeId
│   │   └── Unknown node → Print error → return
│   ├── Get current node using currentNodeIndex
│   │   └── Unavailable → return
│   ├── Does received node match current node?
│   │   └── No → Print unexpected node → return
│   ├── Does sequence match expectedSequence?
│   │   └── No → Print sequence mismatch → return
│   │
│   ├── Validate temperature if FLAG_TEMPERATURE_VALID is set
│   │   └── Non-finite or outside -40 to 85°C → return
│   ├── Validate humidity if FLAG_HUMIDITY_VALID is set
│   │   └── Non-finite or outside 0 to 100% → return
│   ├── Validate pressure if FLAG_PRESSURE_VALID is set
│   │   └── Non-finite or outside 300 to 1100 hPa → return
│   │
│   ├── Mark node online
│   ├── Calculate response time
│   ├── Print received sensor data
│   ├── Is validResponseCallback != nullptr?
│   │   └── Yes → Invoke callback with sensor data
│   └── Call moveToNextNode()
│
├── handleTimeout()
│   ├── Get current node
│   ├── If node exists
│   │   ├── Mark node offline
│   │   └── Print timeout message
│   └── Call moveToNextNode()
│
├── moveToNextNode()
│   ├── Increment currentNodeIndex
│   ├── Is index >= node count?
│   │   └── Yes → Reset currentNodeIndex = 0
│   ├── Set stateTimestamp = millis()
│   └── Set state = IDLE
│
└── setValidResponseCallback(callback)
    └── Store valid response callback