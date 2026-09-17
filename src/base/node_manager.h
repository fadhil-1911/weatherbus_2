#pragma once
#include <Arduino.h>

struct NodeInfo {
    uint8_t nodeId;
    uint8_t mac[6];
    bool online;
    uint32_t lastResponse;
};

class NodeManager {

  public:
    NodeManager();
    void begin();
    uint8_t getNodeCount() const;
    NodeInfo* getNode(uint8_t index);
    NodeInfo* getNodeById(uint8_t nodeId);
    void markOnline(uint8_t nodeId);
    void markOffline(uint8_t nodeId);

  private:
    static constexpr uint8_t MAX_NODES = 2;
    NodeInfo nodes[MAX_NODES];
};