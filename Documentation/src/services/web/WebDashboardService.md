# WebDashboardService — `WebDashboardService.h / .cpp`

## 1. Purpose

Provides a web-based dashboard for WeatherBus_2 to display environmental sensor data from the Base Station's local sensor and a remote sensor node.

The service uses Arduino `WebServer` on port `80` to serve the dashboard HTML and JSON endpoints.

The dashboard automatically refreshes sensor data every 2 seconds without reloading the page.

**Source:**
- `src/services/WebDashboardService.h`
- `src/services/WebDashboardService.cpp`

## 2. Responsibilities

- Start an HTTP web server on port `80`.
- Serve the dashboard interface at `/`.
- Store the latest local sensor readings.
- Store the latest sensor readings for Node 1.
- Provide JSON endpoints for local and remote sensor data.
- Validate sensor values using validity flags and finite-number checks.
- Determine Node 1 online status using a 10-second data-age threshold.
- Prevent browser caching of JSON responses.
- Process incoming HTTP requests through `update()`.
- Return HTTP `404` for unknown endpoints.

## 3. Module Dependencies

| Module / Component | Responsibility |
|---|---|
| `WebDashboardService.h` | Declares the service class, sensor data structure and public interface |
| `WebDashboardService.cpp` | Implements the web server, dashboard HTML and JSON handlers |
| `Arduino.h` | Provides Arduino types and functions such as `millis()` |
| `WebServer.h` | Provides the HTTP server implementation |
| `math.h` | Provides `isfinite()` for sensor value validation |
| `DASHBOARD_HTML` | Stores the dashboard HTML, CSS and JavaScript in program memory |
| Local sensor service | Supplies local sensor data through `setLocalData()` |
| Base Station application | Supplies remote Node data through `setNodeData()` |

## 4. Class Structure

### Public Functions

| Function | Description |
|---|---|
| `begin()` | Registers HTTP routes and starts the web server. |
| `update()` | Processes incoming HTTP requests when the service has started. |
| `setLocalData(temperature, humidity, flags, valid)` | Updates the latest local sensor readings and their validity information. |
| `setNodeData(nodeId, temperature, humidity, pressure, flags)` | Updates the latest readings for Node 1; ignores other node IDs. |

### Private Members

| Member | Description |
|---|---|
| `server_` | HTTP server instance listening on port `80`. |
| `started_` | Indicates whether `begin()` has completed and the service is marked as started. |
| `SensorData` | Structure containing temperature, humidity, pressure, validity flags, validity state and last-update time. |
| `localData_` | Stores the latest local sensor data. |
| `node1Data_` | Stores the latest data received for Node 1. |

### Private Functions

| Function | Description |
|---|---|
| `handleRoot()` | Serves the dashboard HTML page. |
| `handleLocalApi()` | Returns local sensor data as JSON. |
| `handleNodesApi()` | Returns Node 1 data and online status as JSON. |
| `handleNotFound()` | Returns HTTP `404` for unknown endpoints. |

## 5. Initialization — `begin()`

The function registers the following HTTP routes:

| Endpoint | Method | Handler | Purpose |
|---|---|---|---|
| `/` | GET | `handleRoot()` | Serves the dashboard page. |
| `/api/local` | GET | `handleLocalApi()` | Returns local sensor JSON. |
| `/api/nodes` | GET | `handleNodesApi()` | Returns Node 1 JSON. |
| `/local.json` | GET | `handleLocalApi()` | Local sensor endpoint used by the dashboard JavaScript. |
| `/nodes.json` | GET | `handleNodesApi()` | Remote node endpoint used by the dashboard JavaScript. |
| Other paths | Any unmatched request | `handleNotFound()` | Returns HTTP `404`. |

After registering the routes, `begin()`:

1. Starts the HTTP server using `server_.begin()`.
2. Sets `started_ = true`.
3. Returns `true`.

The implementation does not explicitly check for HTTP server startup failure.

## 6. Dashboard Interface

The HTML interface is stored in `DASHBOARD_HTML` using `PROGMEM`.

### Dashboard Sections

| Section | Displayed Information |
|---|---|
| Header | WeatherBus branding and Base Station status |
| Local Sensor | Local temperature and humidity |
| Remote Nodes | Node 1 temperature, humidity and online status |
| System Status | Dashboard readiness and Data API status |
| Footer | WeatherBus_2 local network dashboard label |

The interface uses responsive CSS with a dark colour scheme and a grid layout that adapts to smaller screens.

### Automatic Refresh

The JavaScript function `updateDashboard()`:

1. Requests `/local.json` and `/nodes.json` concurrently.
2. Checks both HTTP responses.
3. Parses the JSON responses.
4. Updates the local temperature and humidity.
5. Updates Node 1 temperature and humidity.
6. Displays Node 1 as `ONLINE` or `OFFLINE`.
7. Sets the Data API status to `ONLINE` when both requests succeed.
8. Sets the Data API status to `ERROR` if a request or parsing operation fails.

The page runs `updateDashboard()` immediately and repeats it every 2 seconds.

Sensor values are displayed to two decimal places. Invalid or unavailable readings are displayed as `--`.

## 7. Local Sensor Data

### `setLocalData()`

Stores the supplied local sensor values:

- Temperature
- Humidity
- Validity flags
- Overall read validity
- Update timestamp using `millis()`

### `handleLocalApi()`

The local JSON handler validates temperature and humidity independently.

Temperature is considered valid when:

- `localData_.valid` is `true`.
- Flag `0x01` is set.
- The temperature is finite.

Humidity is considered valid when:

- `localData_.valid` is `true`.
- Flag `0x02` is set.
- The humidity is finite.

The response contains:

| JSON Field | Description |
|---|---|
| `valid` | Overall local sensor validity state |
| `temperature` | Valid temperature, or `null` |
| `humidity` | Valid humidity, or `null` |
| `age_ms` | Milliseconds elapsed since the last local data update |

The handler sends the response with content type `application/json` and the header `Cache-Control: no-store`.

**Note:** The overall `valid` field represents the supplied sensor-read validity. Individual readings can still be `null` when their respective validity flags are not set or their values are not finite.

## 8. Remote Node Data

### `setNodeData()`

The function accepts data only when `nodeId == 1`.

For Node 1, it stores:

- Temperature
- Humidity
- Pressure
- Validity flags
- Overall validity state, set to `true`
- Update timestamp using `millis()`

Calls for other node IDs are ignored.

### `handleNodesApi()`

Node 1 is considered online when:

- `node1Data_.valid` is `true`.
- The time since its last update is no greater than `10000` milliseconds.

Temperature, humidity and pressure are validated independently:

| Reading | Required Flag | Additional Check |
|---|---|---|
| Temperature | `0x01` | Value must be finite |
| Humidity | `0x02` | Value must be finite |
| Pressure | `0x04` | Value must be finite |

Each reading also requires Node 1 to be online. Invalid readings are returned as `null`.

The response contains:

| JSON Field | Description |
|---|---|
| `node1.online` | Whether Node 1 has valid stored data updated within the last 10 seconds |
| `node1.temperature` | Valid temperature, or `null` |
| `node1.humidity` | Valid humidity, or `null` |
| `node1.pressure` | Valid pressure, or `null` |
| `node1.age_ms` | Milliseconds elapsed since the last Node 1 data update |

The response is sent as `application/json` with `Cache-Control: no-store`.

**Note:** The dashboard interface displays Node 1 temperature, humidity and status. Although the API provides pressure, the current HTML does not display it.

## 9. HTTP Response Behaviour

| Request | Response |
|---|---|
| `GET /` | HTTP `200`, dashboard HTML |
| `GET /api/local` | HTTP `200`, local sensor JSON |
| `GET /local.json` | HTTP `200`, local sensor JSON |
| `GET /api/nodes` | HTTP `200`, Node 1 JSON |
| `GET /nodes.json` | HTTP `200`, Node 1 JSON |
| Unknown endpoint | HTTP `404`, plain-text error message |

The JSON handlers return HTTP `200` even when individual sensor values are invalid or unavailable. Invalid readings are represented using `null`.

## 10. Main Flow Tree

```text
WebDashboardService
│
├── begin()
│   ├── Register GET /
│   │   └── handleRoot()
│   ├── Register GET /api/local
│   │   └── handleLocalApi()
│   ├── Register GET /api/nodes
│   │   └── handleNodesApi()
│   ├── Register GET /local.json
│   │   └── handleLocalApi()
│   ├── Register GET /nodes.json
│   │   └── handleNodesApi()
│   ├── Register onNotFound()
│   │   └── handleNotFound()
│   ├── server_.begin()
│   ├── started_ = true
│   └── Return true
│
├── setLocalData(...)
│   ├── Store temperature
│   ├── Store humidity
│   ├── Store flags and validity
│   └── Update lastUpdate
│
├── setNodeData(...)
│   ├── Check nodeId == 1
│   │   └── Otherwise → Return
│   ├── Store temperature, humidity and pressure
│   ├── Store flags
│   ├── Set valid = true
│   └── Update lastUpdate
│
├── update()
│   └── If started_
│       └── server_.handleClient()
│
├── handleRoot()
│   └── Send dashboard HTML
│
├── handleLocalApi()
│   ├── Validate local temperature
│   ├── Validate local humidity
│   ├── Calculate age_ms
│   ├── Build JSON response
│   ├── Set Cache-Control: no-store
│   └── Send HTTP 200
│
├── handleNodesApi()
│   ├── Calculate Node 1 online status
│   ├── Validate temperature, humidity and pressure
│   ├── Calculate age_ms
│   ├── Build JSON response
│   ├── Set Cache-Control: no-store
│   └── Send HTTP 200
│
└── handleNotFound()
    └── Send HTTP 404
```

## 11. Dashboard Data Flow

```text
Base Station Application
          │
          ├── Local Sensor Reading
          │       │
          │       ▼
          │   setLocalData(...)
          │       │
          │       ▼
          │   localData_
          │
          └── Remote Node Response
                  │
                  ▼
              setNodeData(...)
                  │
                  ▼
              node1Data_

Browser
   │
   ├── GET /
   │     └── Dashboard HTML
   │
   ├── GET /local.json
   │     └── handleLocalApi()
   │            └── Local Sensor JSON
   │
   └── GET /nodes.json
         └── handleNodesApi()
                └── Node 1 JSON

Dashboard JavaScript
   │
   ├── Update local temperature and humidity
   ├── Update Node 1 temperature and humidity
   ├── Update Node 1 online status
   └── Update Data API status

Refresh interval: 2000 ms
```