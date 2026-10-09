// =====================================================
// File: src/drivers/ESPNowTransport.h
// =====================================================

#pragma once

#include "../interfaces/ITransport.h"

class ESPNowTransport : public ITransport
{
public:
    ESPNowTransport();

    bool begin(
        TransportReceiveCallback callback) override;

    bool addPeer(
        const TransportAddress& address) override;

    bool send(
        const TransportAddress& destination,
        const uint8_t* data,
        size_t length) override;

    void update() override;

    bool isReady() const override;

private:
    static constexpr uint8_t ESP_NOW_ADDRESS_LENGTH = 6;

    TransportReceiveCallback receiveCallback;

    bool ready;

    static ESPNowTransport* instance;

    static void onDataReceived(
        const uint8_t* mac,
        const uint8_t* data,
        int len);

    void handleReceive(
        const uint8_t* data,
        size_t length);
};