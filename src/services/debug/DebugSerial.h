// =====================================================
// File: src/services/debug/DebugSerial.h
// =====================================================

#pragma once

#include <Arduino.h>

class DebugSerialClass : public Print {
public:
    void begin(unsigned long baudRate = 115200);

    void setClient(class WiFiClient* client);
    void update();

    size_t write(uint8_t data) override;
    size_t write(const uint8_t* buffer, size_t size) override;

    using Print::write;

private:
    static constexpr size_t BUFFER_SIZE = 4096;

    uint8_t buffer[BUFFER_SIZE];

    volatile size_t head = 0;
    volatile size_t tail = 0;

    class WiFiClient* client = nullptr;
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    void enqueue(uint8_t data);
};

extern DebugSerialClass DebugSerial;

#if defined(WEATHERBUS_BASE)
#define Serial DebugSerial
#endif