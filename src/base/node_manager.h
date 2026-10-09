
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



/*
// =====================================================
// File: src/base/node_manager.h
// =====================================================

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
    static constexpr uint8_t MAX_NODES = 1; // FOR TESTING PURPOSES, ONLY ONE NODE IS CONFIGURED. CHANGE THIS TO 2 OR MORE FOR PRODUCTION.
    NodeInfo nodes[MAX_NODES];
}; */