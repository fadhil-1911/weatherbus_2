// =====================================================
// File: src/services/NodeCommunicationService.h
// =====================================================

#pragma once

#include "../../interfaces/ITransport.h"
#include "../../interfaces/ISensorSource.h"

class NodeCommunicationService {
  public:
    NodeCommunicationService(
        ITransport& transport,
        ISensorSource& bme280Source,
        ISensorSource& sht41Source);

    bool begin(const TransportAddress& baseAddress);
    void update();

  private:
    ITransport& transport;
    ISensorSource& bme280Source;
    ISensorSource& sht41Source;

    TransportAddress baseAddress;
    bool ready = false;

    bool requestPending = false;
    uint16_t pendingSequence = 0;

    static NodeCommunicationService* instance;

    static void onDataReceived(
        const uint8_t* data,
        size_t length);

    void handleReceivedData(
        const uint8_t* data,
        size_t length);

    void processRequest(uint16_t sequence);
};
