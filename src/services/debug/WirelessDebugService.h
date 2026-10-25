// =====================================================
// File: src/services/debug/WirelessDebugService.h
// =====================================================

#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WirelessDebugService {
public:
    bool begin();
    void update();

private:
    static constexpr uint16_t TCP_PORT = 3333;
    static constexpr uint8_t WIFI_CHANNEL = 1;

    const char* apSSID = "WeatherBus-Debug";
    const char* apPassword = "WB2Debug";

    WiFiServer server{TCP_PORT};
    WiFiClient client;
};