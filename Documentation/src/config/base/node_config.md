# Base Station — Node Configuration

## 1. Purpose

Defines the Sensor Nodes known to the WeatherBus Base Station.

Each entry associates a Node ID with its ESP-NOW MAC address.

**Source:** `src/config/base/node_config.h`

## 2. Configuration Structure

The `NodeConfig` structure contains:

| Field | Type | Description |
|---|---|---|
| `nodeId` | `uint8_t` | Unique identifier assigned to a Sensor Node |
| `mac` | `uint8_t[6]` | Six-byte MAC address of the Sensor Node |

## 3. Configured Nodes

The current configuration contains one active Node.

| Node ID | MAC Address | Status |
|---|---|---|
| 1 | `9C:CC:01:7D:14:80` | Enabled |

Node 2 is present as a commented-out example and is not included in
the active configuration.

## 4. Node Count

`NODE_COUNT` is calculated automatically from the number of entries
in `NODE_CONFIGS`.

This avoids manually maintaining a separate Node count when entries
are added or removed.

## 5. Adding a Node

To register another Sensor Node:

1. Obtain the Node's actual MAC address.
2. Assign a unique Node ID.
3. Add an entry to `NODE_CONFIGS`.
4. Verify that the Node firmware uses the corresponding Node ID.
5. Build and test the Base Station and Node firmware.

Example:

    {2, {0x9C, 0xCC, 0x01, 0x7D, 0x14, 0x81}}

The example address must be replaced if it does not match the actual
hardware MAC address.

## 6. Development Constraints

- Node IDs must be unique within the configuration.
- Each MAC address must correspond to the intended Sensor Node.
- The Base Station and Node firmware must agree on the Node ID.
- `NODE_COUNT` must reflect the active configuration.
- Node registration and ESP-NOW peer setup should be verified after
  changing the configuration.

## 7. Implementation Notes

This file defines the Node configuration data.

It does not itself initialize ESP-NOW, register peers, poll Nodes or
validate received sensor data. Those operations are handled by other
modules.

## 8. Test Status

This configuration file alone does not establish that every configured
Node has been tested successfully.

Record hardware communication and multi-Node test results in the
project testing documentation.