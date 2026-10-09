// =====================================================
// File: src/drivers/ESPNowTransport.cpp
// =====================================================

#include "ESPNowTransport.h"

#include <WiFi.h>
#include <esp_now.h>

ESPNowTransport* ESPNowTransport::instance = nullptr;

// =====================================================
// Constructor
// =====================================================

ESPNowTransport::ESPNowTransport()
    : receiveCallback(nullptr),
      ready(false) {
    instance = this;
}

// =====================================================
// Begin
// =====================================================

bool ESPNowTransport::begin(
    TransportReceiveCallback callback) {
    receiveCallback = callback;

    WiFi.mode(WIFI_STA);

    Serial.print("Base MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println(
            "ERROR: ESP-NOW initialization failed");

        ready = false;
        return false;
    }

    esp_now_register_recv_cb(
        ESPNowTransport::onDataReceived);

    ready = true;

    Serial.println(
        "ESP-NOW transport initialized");

    return true;
}

// =====================================================
// Add Peer
// =====================================================

bool ESPNowTransport::addPeer(
    const TransportAddress& address) {
    if (!ready) {
        Serial.println(
            "ERROR: ESP-NOW transport not ready");
        return false;
    }

    if (address.length != ESP_NOW_ADDRESS_LENGTH) {
        Serial.println(
            "ERROR: Invalid ESP-NOW address length");
        return false;
    }

    esp_now_peer_info_t peerInfo{};

    memcpy(
        peerInfo.peer_addr,
        address.data,
        ESP_NOW_ADDRESS_LENGTH);

    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    esp_err_t result =
        esp_now_add_peer(&peerInfo);

    if (result != ESP_OK) {
        Serial.printf(
            "ERROR: Failed to add ESP-NOW peer | %d\n",
            result);

        return false;
    }

    Serial.println(
        "ESP-NOW peer added");

    return true;
}

// =====================================================
// Send
// =====================================================

bool ESPNowTransport::send(
    const TransportAddress& destination,
    const uint8_t* data,
    size_t length) {
    if (!ready) {
        Serial.println(
            "ERROR: ESP-NOW transport not ready");
        return false;
    }

    if (destination.length != ESP_NOW_ADDRESS_LENGTH) {
        Serial.println(
            "ERROR: Invalid ESP-NOW destination");
        return false;
    }

    if (data == nullptr || length == 0) {
        Serial.println(
            "ERROR: Invalid transport data");
        return false;
    }

    esp_err_t result =
        esp_now_send(
            destination.data,
            data,
            length);

    if (result != ESP_OK) {
        Serial.printf(
            "ESP-NOW TX ERROR: %d\n",
            result);
        return false;
    }
    return true;
}

// =====================================================
// Update
// =====================================================

void ESPNowTransport::update() {
    // ESP-NOW is callback-driven.
    // No periodic processing is required here.
}

// =====================================================
// Status
// =====================================================

bool ESPNowTransport::isReady() const {
    return ready;
}

// =====================================================
// ESP-NOW RX Callback
// =====================================================

void ESPNowTransport::onDataReceived(
    const uint8_t* mac,
    const uint8_t* data,
    int len) {
    (void)mac;

    if (instance == nullptr) {
        return;
    }

    if (data == nullptr || len <= 0) {
        return;
    }

    instance->handleReceive(
        data,
        static_cast<size_t>(len));
}

// =====================================================
// Handle RX
// =====================================================

void ESPNowTransport::handleReceive(
    const uint8_t* data,
    size_t length) {
    if (receiveCallback == nullptr) {
        return;
    }

    receiveCallback(
        data,
        length);
}