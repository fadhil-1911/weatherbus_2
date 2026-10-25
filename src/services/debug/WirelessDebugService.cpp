// =====================================================
// File: src/services/debug/WirelessDebugService.cpp
// =====================================================

#include "WirelessDebugService.h"
#include "DebugSerial.h"

#include <WiFi.h>

#undef Serial

// Wi-Fi mode selection
static constexpr bool USE_AP_MODE = false;

// Existing AP configuration
const char* apSSID = "WeatherBus-Debug";
const char* apPassword = "WB2Debug88";

// STA configuration — change to your router credentials
const char* staSSID = "iPhone Fadhil & Mardiana";
const char* staPassword = "12345678";

// =====================================================
// Begin
// =====================================================

bool WirelessDebugService::begin() {
    if (USE_AP_MODE) {
        // AP mode — existing behaviour
        WiFi.mode(WIFI_AP_STA);

        if (!WiFi.softAP(
                apSSID,
                apPassword,
                WIFI_CHANNEL)) {
            DebugSerial.println(
                "[WiFi Debug] Failed to start AP");
            return false;
        }

        DebugSerial.println("[WiFi] Mode: AP");
        DebugSerial.print("SSID: ");
        DebugSerial.println(apSSID);
        DebugSerial.print("IP: ");
        DebugSerial.println(WiFi.softAPIP());

    } else {
        // STA mode — join existing router
        WiFi.mode(WIFI_STA);
        WiFi.begin(staSSID, staPassword);

        DebugSerial.println("[WiFi] Mode: STA");
        DebugSerial.print("Connecting to: ");
        DebugSerial.println(staSSID);

        const unsigned long startTime = millis();
        constexpr unsigned long CONNECT_TIMEOUT_MS = 15000;

        while (WiFi.status() != WL_CONNECTED &&
               millis() - startTime < CONNECT_TIMEOUT_MS) {
            delay(250);
        }

        if (WiFi.status() != WL_CONNECTED) {
            DebugSerial.println(
                "[WiFi] STA connection failed");
            return false;
        }

        DebugSerial.println("[WiFi] STA connected");
        DebugSerial.print("IP: ");
        DebugSerial.println(WiFi.localIP());
    }

    server.begin();
    server.setNoDelay(true);

    DebugSerial.print("TCP Debug Port: ");
    DebugSerial.println(TCP_PORT);

    return true;
}

// =====================================================
// Update
// =====================================================

void WirelessDebugService::update() {
    if (server.hasClient()) {
        WiFiClient newClient = server.accept();

        if (newClient) {
            if (client && client.connected()) {
                client.stop();
            }

            client = newClient;
            client.setNoDelay(true);

            DebugSerial.setClient(&client);
            DebugSerial.println("[WiFi Debug] Client connected");
        }
    }

    if (client && !client.connected()) {
        client.stop();
        DebugSerial.setClient(nullptr);
    }

    DebugSerial.update();
}