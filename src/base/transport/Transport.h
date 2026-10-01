#pragma once

#include <Arduino.h>

struct TransportAddress
{
    static constexpr uint8_t MAX_LENGTH = 8;

    uint8_t data[MAX_LENGTH];
    uint8_t length;

    TransportAddress()
        : data{},
          length(0)
    {
    }
};

using TransportReceiveCallback =
    void (*)(const uint8_t* data, size_t length);

class Transport
{
public:
    virtual ~Transport() = default;

    virtual bool begin(
        TransportReceiveCallback callback) = 0;

    virtual bool addPeer(
        const TransportAddress& address) = 0;

    virtual bool send(
        const TransportAddress& destination,
        const uint8_t* data,
        size_t length) = 0;

    virtual void update() = 0;

    virtual bool isReady() const = 0;
};