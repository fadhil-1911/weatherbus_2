// =====================================================
// File: src/services/debug/WirelessDebugService.cpp
// =====================================================

#include "WirelessDebugService.h"
#include "DebugSerial.h"
#include "DebugSerial.h"

#include <WiFi.h>

#undef Serial

// =====================================================
// Begin
// =====================================================

bool WirelessDebugService::begin() {
    WiFi.mode(WIFI_AP_STA);

    if (!WiFi.softAP(
            apSSID,
            apPassword,
            WIFI_CHANNEL)) {

        DebugSerial.println(
            "[WiFi Debug] Failed to start access point");

        return false;
    }

    server.begin();
    server.setNoDelay(true);

    DebugSerial.println();
    DebugSerial.println(
        "========== WIRELESS DEBUG ==========");

    DebugSerial.print("SSID: ");
    DebugSerial.println(apSSID);

    DebugSerial.print("IP: ");
    DebugSerial.println(WiFi.softAPIP());

    DebugSerial.print("TCP Port: ");
    DebugSerial.println(TCP_PORT);

    DebugSerial.println(
        "Waiting for debug client...");

    DebugSerial.println(
        "====================================");

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