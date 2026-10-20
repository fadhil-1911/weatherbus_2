# NodeManager

**Source:** `src/base/node_manager.cpp`

## Purpose

`NodeManager` manages the Base Station's configured sensor nodes, including their IDs, MAC addresses, online status, and last response timestamps.

## Responsibilities

- Initialize node information from `NODE_CONFIGS`.
- Display configured nodes through Serial output.
- Provide access to nodes by index or node ID.
- Update a node's online status.
- Record the time when a node is marked online.

## Initialization

The constructor initializes each configured node with:

- Node ID and MAC address copied from `NODE_CONFIGS`.
- `online = false`.
- `lastResponse = 0`.

`begin()` prints the Node Manager heading and lists the configured node IDs. It does not perform network communication or check whether nodes are reachable.

## Main Functions

### `getNodeCount()`

Returns `NODE_COUNT`, the number of configured nodes.

### `getNode(index)`

Returns a pointer to the node at the specified index.

Returns `nullptr` if the index is outside the configured node range.

### `getNodeById(nodeId)`

Searches for a node with the specified ID.

Returns a pointer to the matching node, or `nullptr` if no match exists.

### `markOnline(nodeId)`

If the node exists:

- Sets `online = true`.
- Updates `lastResponse` using `millis()`.

Unknown node IDs are ignored.

### `markOffline(nodeId)`

If the node exists, sets `online = false`.

Unknown node IDs are ignored. The function does not reset `lastResponse`.

## Dependencies

- `node_manager.h`
- `NODE_CONFIGS`
- `NODE_COUNT`
- Arduino `Serial` and `millis()`
- `memcpy()`

## Behavioural Notes

- Node information is initialized from the compile-time configuration.
- Online status changes only when the relevant functions are called.
- This implementation does not automatically detect offline nodes or apply a timeout.
- `lastResponse` records the time a node is marked online; it is not updated by `markOffline()`.

## Test Status

**Code review:** Behaviour documented from the implementation.

Runtime and hardware test status must be confirmed separately.