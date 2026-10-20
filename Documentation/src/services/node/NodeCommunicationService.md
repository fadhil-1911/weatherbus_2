# NodeCommunicationService

**Source:** `src/services/node/NodeCommunicationService.cpp`

## Purpose

`NodeCommunicationService` manages communication between a sensor Node and the Base Station. It receives `DATA_REQUEST` packets, reads configured sensors, constructs `SENSOR_DATA` packets, and sends responses through the `ITransport` interface.

## Responsibilities

- Initialize transport communication and register the Base Station as a peer.
- Validate incoming `DATA_REQUEST` packets.
- Store pending requests for processing in the main loop.
- Read measurements from BME280 and SHT41 sensor sources.
- Select temperature and humidity according to node configuration.
- Set measurement validity flags.
- Construct and transmit `SENSOR_DATA` packets.

## Initialization

### `begin(baseAddress)`

1. Stores the Base Station address.
2. Registers the service instance for the transport receive callback.
3. Initializes the transport.
4. Adds the Base Station as a peer.
5. Sets `ready = true` when initialization succeeds.

Returns `false` if transport initialization or peer registration fails.

If transport initialization fails, the service instance pointer is reset. If peer registration fails, `ready` is set to `false`.

## Request Handling

### `onDataReceived(data, length)`

Static receive callback that forwards incoming data to the active service instance.

### `handleReceivedData(data, length)`

Validates incoming requests by checking:

- The data pointer is not null.
- Packet length matches `sizeof(Header)`.
- Protocol version matches `PROTOCOL_VERSION`.
- Packet type is `DATA_REQUEST`.
- Node ID matches `NODE_ID`.
- Payload length is zero.

Accepted requests store the sequence number in a pending-request buffer protected by a critical section.

If a request is already pending, the new request is ignored.

### `update()`

Processes pending requests from the main loop.

If the service is not ready, it returns immediately. Otherwise, it retrieves and clears the pending request under a critical section, then calls `processRequest()`.

## Sensor Acquisition

### `processRequest(requestSequence)`

Reads sensor measurements according to the configuration in `node_settings.h`.

**BME280**
- Read when `ENABLE_BME280` is enabled.
- Pressure is copied from the BME280 reading.
- Pressure validity is determined by `pressureValid`.
- Temperature is used only when `USE_BME280_TEMPERATURE` is enabled and `temperatureValid` is true.
- Humidity is used only when `USE_BME280_HUMIDITY` is enabled and `humidityValid` is true.

**SHT41**
- Read when `ENABLE_SHT41` is enabled.
- Supplies temperature when `USE_BME280_TEMPERATURE` is disabled and the SHT41 temperature reading is valid.
- Supplies humidity when `USE_BME280_HUMIDITY` is disabled and the SHT41 humidity reading is valid.

The service sets the corresponding validity flags for accepted measurements.

If a sensor read fails, a warning is printed. Processing continues, and the service may still send a response with the measurements that remain valid.

## Packet Construction

The service constructs a `SensorDataPacket` containing:

- Protocol version.
- Packet type `SENSOR_DATA`.
- Node ID.
- Sensor validity flags.
- Original request sequence number.
- Payload length set to `sizeof(SensorDataPayload)`.
- CRC set to zero.
- Reserved field set to zero.
- Temperature, humidity, and pressure values.

The packet is sent to the configured Base Station address through `ITransport`.

A successful return from `transport.send()` is logged as a successful send request. A failed send produces an error message.

## Dependencies

- `NodeCommunicationService.h`
- `ITransport`
- `ISensorSource`
- `node_settings.h`
- `weatherbus_protocol.h`
- Arduino and FreeRTOS critical-section primitives

## Behavioural Notes and Limitations

- Only one request can be pending at a time. Additional requests received while one is pending are ignored.
- Sensor readings are processed when `update()` handles the pending request, not directly inside the receive callback.
- CRC is disabled and set to zero.
- The request validation does not explicitly check the request's flags, reserved field, or CRC.
- Pressure is copied from the BME280 reading even when `pressureValid` is false. In that case, the pressure validity flag remains unset.
- SHT41 is used as a temperature or humidity source only when the corresponding BME280 selection option is disabled.
- The service does not automatically retry failed transmissions.
- The implementation does not confirm successful delivery to the Base Station.

## Test Status

**Code review:** Request validation, sensor selection, packet construction, and transport handling are documented from the implementation.

Hardware and integration test results should be recorded separately.