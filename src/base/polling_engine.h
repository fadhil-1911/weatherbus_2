#pragma once

#include <Arduino.h>
#include "node_manager.h"

class PollingEngine {
  public:
    PollingEngine(NodeManager& manager);

    void begin();
    void update();

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

    static constexpr uint32_t RESPONSE_TIMEOUT_MS = 200;
    static constexpr uint32_t POLL_INTERVAL_MS = 2000;
    static constexpr uint32_t STATE_DELAY_MS = 10;

    void sendRequest();
    void moveToNextNode();
    void handleTimeout();
};