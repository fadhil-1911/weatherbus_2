// =====================================================
// File: src/services/debug/DebugSerial.cpp
// =====================================================

#include "DebugSerial.h"

#include <WiFi.h>

// Use the real Arduino Serial.
// Do not use the DebugSerial macro.
#undef Serial

DebugSerialClass DebugSerial;

// =====================================================
// Begin
// =====================================================

void DebugSerialClass::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
}

// =====================================================
// Set TCP Client
// =====================================================

void DebugSerialClass::setClient(WiFiClient* newClient) {
    client = newClient;
}

// =====================================================
// Write One Byte
//
// USB output remains unchanged.
// A copy is added to the Wi-Fi buffer.
// =====================================================

size_t DebugSerialClass::write(uint8_t data) {
    Serial.write(data);
    enqueue(data);

    return 1;
}

// =====================================================
// Write Buffer
// =====================================================

size_t DebugSerialClass::write(
    const uint8_t* data,
    size_t size) {

    if (data == nullptr || size == 0) {
        return 0;
    }

    Serial.write(data, size);

    for (size_t i = 0; i < size; i++) {
        enqueue(data[i]);
    }

    return size;
}

// =====================================================
// Enqueue
//
// If the buffer is full, the new byte is dropped.
// The program does not wait for the Wi-Fi connection.
// =====================================================

void DebugSerialClass::enqueue(uint8_t data) {
    portENTER_CRITICAL(&mux);

    size_t next = (head + 1) % BUFFER_SIZE;

    if (next != tail) {
        buffer[head] = data;
        head = next;
    }

    portEXIT_CRITICAL(&mux);
}

// =====================================================
// Update
//
// Called periodically from loop().
// TCP transmission is not performed inside write().
// =====================================================

void DebugSerialClass::update() {
    if (client == nullptr || !client->connected()) {
        return;
    }

    uint8_t chunk[256];
    size_t count = 0;

    portENTER_CRITICAL(&mux);

    size_t availableInBuffer = (head >= tail)
                                   ? (head - tail)
                                   : (BUFFER_SIZE - tail + head);

    count = availableInBuffer;

    if (count > sizeof(chunk)) {
        count = sizeof(chunk);
    }

    for (size_t i = 0; i < count; i++) {
        chunk[i] = buffer[(tail + i) % BUFFER_SIZE];
    }

    portEXIT_CRITICAL(&mux);

    if (count == 0) {
        return;
    }

    // Send directly without relying on availableForWrite().
    size_t sent = client->write(chunk, count);

    if (sent > 0) {
        portENTER_CRITICAL(&mux);
        tail = (tail + sent) % BUFFER_SIZE;
        portEXIT_CRITICAL(&mux);
    }
}