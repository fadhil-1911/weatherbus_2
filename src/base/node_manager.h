
// =====================================================
// File: src/base/node_manager.h
// =====================================================

#pragma once

#include <Arduino.h>
//#include "../config/node_config.h"
#include "config/node_config.h"

struct NodeInfo
{
    uint8_t nodeId;
    uint8_t mac[6];
    bool online;
    uint32_t lastResponse;
};

class NodeManager
{
public:
    NodeManager();

    void begin();
    uint8_t getNodeCount() const;
    NodeInfo* getNode(uint8_t index);
    NodeInfo* getNodeById(uint8_t nodeId);
    void markOnline(uint8_t nodeId);
    void markOffline(uint8_t nodeId);

private:
    NodeInfo nodes[NODE_COUNT];
};



