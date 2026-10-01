// =====================================================
// File: src/base/polling_engine.h
// =====================================================

#pragma once

#include <Arduino.h>
#include "node_manager.h"
#include "transport/Transport.h"

class PollingEngine {
  public:
    PollingEngine(
        NodeManager& manager,
        Transport& transport);

    void begin();
    void update();

    // API kekal sama - tidak perlu ubah logic application
    void onSensorData(
        uint8_t nodeId,
        uint16_t sequence,
        uint8_t flags,
        float temperature,
        float humidity,
        float pressure);

  private:
    enum class State {
        IDLE,
        SEND_REQUEST,
        WAIT_RESPONSE
    };

    NodeManager& nodeManager;
    Transport& transport;

    State state;

    uint8_t currentNodeIndex;

    uint16_t sequenceNumber;
    uint16_t expectedSequence;

    uint32_t stateTimestamp;
    uint32_t requestTimestamp;

    // -------------------------------------------------
    // Pending response
    //
    // Transport callback hanya menyimpan data di sini.
    // Processing sebenar dibuat dalam update().
    // -------------------------------------------------

    volatile bool responsePending;

    uint8_t pendingNodeId;
    uint16_t pendingSequence;
    uint8_t pendingFlags;

    float pendingTemperature;
    float pendingHumidity;
    float pendingPressure;

    static constexpr uint32_t RESPONSE_TIMEOUT_MS = 200;
    static constexpr uint32_t POLL_INTERVAL_MS = 2000;
    static constexpr uint32_t STATE_DELAY_MS = 10;

    void sendRequest();
    void processPendingResponse();
    void moveToNextNode();
    void handleTimeout();
};

/*/ espnow version 
// =====================================================
// File: src/base/polling_engine.h
// =====================================================


#pragma once

#include <Arduino.h>
#include "node_manager.h"

class PollingEngine {
  public:
    PollingEngine(NodeManager& manager);

    void begin();
    void update();

    // API kekal sama - tidak perlu ubah main.cpp
    void onSensorData(
        uint8_t nodeId,
        uint16_t sequence,
        uint8_t flags,
        float temperature,
        float humidity,
        float pressure);

  private:
    enum class State {
        IDLE,
        SEND_REQUEST,
        WAIT_RESPONSE
    };

    NodeManager& nodeManager;

    State state;

    uint8_t currentNodeIndex;

    uint16_t sequenceNumber;
    uint16_t expectedSequence;

    uint32_t stateTimestamp;
    uint32_t requestTimestamp;

    // -------------------------------------------------
    // Pending response
    //
    // ESP-NOW callback hanya menyimpan data di sini.
    // Processing sebenar dibuat dalam update().
    // -------------------------------------------------

    volatile bool responsePending;

    uint8_t pendingNodeId;
    uint16_t pendingSequence;
    uint8_t pendingFlags;

    float pendingTemperature;
    float pendingHumidity;
    float pendingPressure;

    static constexpr uint32_t RESPONSE_TIMEOUT_MS = 200;
    static constexpr uint32_t POLL_INTERVAL_MS = 2000;
    static constexpr uint32_t STATE_DELAY_MS = 10;

    void sendRequest();
    void processPendingResponse();
    void moveToNextNode();
    void handleTimeout();
}; */
