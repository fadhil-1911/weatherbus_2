// =====================================================
// File: src/services/web/WebDashboardService.cpp
// =====================================================

#include "WebDashboardService.h"
#include <math.h>

namespace {

const char DASHBOARD_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#101827">

<title>WeatherBus | Dashboard</title>

<style>
:root {
    color-scheme: dark;
    --bg: #101827;
    --panel: #1b2638;
    --border: #334155;
    --text: #e5edf7;
    --muted: #9aaac0;
    --accent: #54d6a0;
}

* {
    box-sizing: border-box;
}

body {
    margin: 0;
    background: var(--bg);
    color: var(--text);
    font-family: Arial, sans-serif;
}

header {
    padding: 22px;
    border-bottom: 1px solid var(--border);
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 12px;
    flex-wrap: wrap;
}

.brand {
    font-size: 21px;
    font-weight: 700;
}

.subtitle {
    color: var(--muted);
    font-size: 13px;
    margin-top: 6px;
}

.status {
    color: var(--accent);
    font-size: 13px;
}

main {
    max-width: 1100px;
    margin: auto;
    padding: 22px;
}

h2 {
    font-size: 18px;
    margin: 8px 0 16px;
}

.grid {
    display: grid;
    grid-template-columns:
        repeat(auto-fit, minmax(210px, 1fr));
    gap: 15px;
}

.card {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 20px;
    min-height: 135px;
}

.label {
    color: var(--muted);
    font-size: 14px;
}

.value {
    font-size: 30px;
    font-weight: 700;
    margin-top: 18px;
}

.unit {
    font-size: 15px;
    color: var(--muted);
    font-weight: 400;
}

.section {
    margin-top: 30px;
}

footer {
    color: var(--muted);
    font-size: 12px;
    padding: 24px;
    text-align: center;
}

@media (max-width: 480px) {
    header, main {
        padding: 16px;
    }

    .value {
        font-size: 27px;
    }
}
</style>
</head>

<body>
<header>
    <div>
        <div class="brand">WeatherBus</div>
        <div class="subtitle">Environmental Monitoring System</div>
    </div>
    <div class="status">● BASE STATION</div>
</header>

<main>
    <h2>Local Sensor</h2>

    <div class="grid">
        <section class="card">
            <div class="label">Temperature</div>
            <!-- Local temperature -->
<div class="value">
    <span id="local-temperature">--</span>
    <span class="unit">°C</span>
</div>
        </section>

        <section class="card">
            <div class="label">Humidity</div>
            <!-- Local humidity -->
<div class="value">
    <span id="local-humidity">--</span>
    <span class="unit">%</span>
</div>
        </section>
    </div>

    <div class="section">
        <h2>Remote Nodes</h2>

        <div class="grid">
<section class="card">
    <div class="label">Node 1</div>

    <div class="value">
        <span id="node1-temperature">--</span>
        <span class="unit">°C</span>
    </div>

    <div class="label" style="margin-top: 12px">
        Humidity:
        <span id="node1-humidity">--</span> %

        <span id="node1-status">WAITING</span>
    </div>
</section>
        </div>
    </div>

    <div class="section">
        <h2>System Status</h2>

        <div class="grid">
            <section class="card">
                <div class="label">Dashboard</div>
                <div class="value">READY</div>
            </section>

            <section class="card">
                <div class="label">Data API</div>
                <div class="value" id="api-status">PENDING</div>
            </section>
        </div>
    </div>
</main>

<footer>WeatherBus_2 · Local Network Dashboard</footer>

<script>
async function updateDashboard() {
    const apiStatus = document.getElementById('api-status');

    try {
        const [localResponse, nodesResponse] =
            await Promise.all([
                fetch('/local.json', { cache: 'no-store' }),
                fetch('/nodes.json', { cache: 'no-store' })
            ]);

        if (!localResponse.ok || !nodesResponse.ok) {
            throw new Error('API request failed');
        }

        const local = await localResponse.json();
        const nodes = await nodesResponse.json();
        const node = nodes.node1;

        // Local SHT41
        document.getElementById('local-temperature').textContent =
            local.valid && local.temperature != null
                ? Number(local.temperature).toFixed(2)
                : '--';

        document.getElementById('local-humidity').textContent =
            local.valid && local.humidity != null
                ? Number(local.humidity).toFixed(2)
                : '--';

        // Remote Node 1
        const online = node && node.online === true;

        document.getElementById('node1-temperature').textContent =
            online && node.temperature != null
                ? Number(node.temperature).toFixed(2)
                : '--';

        document.getElementById('node1-humidity').textContent =
            online && node.humidity != null
                ? Number(node.humidity).toFixed(2)
                : '--';

        document.getElementById('node1-status').textContent =
            online ? 'ONLINE' : 'OFFLINE';

        apiStatus.textContent = 'ONLINE';

    } catch (error) {
        apiStatus.textContent = 'ERROR';
        console.error('Dashboard API error:', error);
    }
}

updateDashboard();
setInterval(updateDashboard, 2000);
</script>
</body>
</html>

)HTML";

} // namespace

bool WebDashboardService::begin() {
    server_.on("/", HTTP_GET, [this]() {
        handleRoot();
    });

    server_.on("/api/local", HTTP_GET, [this]() {
        handleLocalApi();
    });

    server_.on("/api/nodes", HTTP_GET, [this]() {
        handleNodesApi();
    });

    server_.onNotFound([this]() {
        handleNotFound();
    });

    server_.on("/local.json", HTTP_GET, [this]() {
        handleLocalApi();
    });

    server_.on("/nodes.json", HTTP_GET, [this]() {
        handleNodesApi();
    });

    server_.begin();
    started_ = true;

    return true;
}

void WebDashboardService::setLocalData(
    float temperature,
    float humidity,
    uint8_t flags,
    bool valid) {

    localData_.temperature = temperature;
    localData_.humidity = humidity;
    localData_.flags = flags;
    localData_.valid = valid;
    localData_.lastUpdate = millis();
}

void WebDashboardService::setNodeData(
    uint8_t nodeId,
    float temperature,
    float humidity,
    float pressure,
    uint8_t flags) {

    if (nodeId != 1) {
        return;
    }

    node1Data_.temperature = temperature;
    node1Data_.humidity = humidity;
    node1Data_.pressure = pressure;
    node1Data_.flags = flags;
    node1Data_.valid = true;
    node1Data_.lastUpdate = millis();
}

void WebDashboardService::handleLocalApi() {
    char json[256];

    const bool temperatureValid =
        localData_.valid &&
        (localData_.flags & 0x01) &&
        isfinite(localData_.temperature);

    const bool humidityValid =
        localData_.valid &&
        (localData_.flags & 0x02) &&
        isfinite(localData_.humidity);

    snprintf(
        json,
        sizeof(json),
        "{\"valid\":%s,"
        "\"temperature\":%s,"
        "\"humidity\":%s,"
        "\"age_ms\":%lu}",
        localData_.valid ? "true" : "false",
        temperatureValid ? String(localData_.temperature, 2).c_str() : "null",
        humidityValid ? String(localData_.humidity, 2).c_str() : "null",
        static_cast<unsigned long>(
            millis() - localData_.lastUpdate));

    server_.sendHeader("Cache-Control", "no-store");
    server_.send(
        200,
        "application/json",
        json);
}

void WebDashboardService::handleNodesApi() {
    char json[320];

    const bool nodeOnline =
        node1Data_.valid &&
        (millis() - node1Data_.lastUpdate <= 10000);

    const bool temperatureValid =
        nodeOnline &&
        (node1Data_.flags & 0x01) &&
        isfinite(node1Data_.temperature);

    const bool humidityValid =
        nodeOnline &&
        (node1Data_.flags & 0x02) &&
        isfinite(node1Data_.humidity);

    const bool pressureValid =
        nodeOnline &&
        (node1Data_.flags & 0x04) &&
        isfinite(node1Data_.pressure);

    snprintf(
        json,
        sizeof(json),
        "{\"node1\":{\"online\":%s,"
        "\"temperature\":%s,"
        "\"humidity\":%s,"
        "\"pressure\":%s,"
        "\"age_ms\":%lu}}",
        nodeOnline ? "true" : "false",
        temperatureValid ? String(node1Data_.temperature, 2).c_str() : "null",
        humidityValid ? String(node1Data_.humidity, 2).c_str() : "null",
        pressureValid ? String(node1Data_.pressure, 2).c_str() : "null",
        static_cast<unsigned long>(
            millis() - node1Data_.lastUpdate));

    server_.sendHeader("Cache-Control", "no-store");
    server_.send(
        200,
        "application/json",
        json);
}

void WebDashboardService::update() {
    if (started_) {
        server_.handleClient();
    }
}

void WebDashboardService::handleRoot() {
    server_.send_P(
        200,
        "text/html; charset=utf-8",
        DASHBOARD_HTML);
}

void WebDashboardService::handleNotFound() {
    server_.send(
        404,
        "text/plain",
        "WeatherBus: endpoint not found");
}

