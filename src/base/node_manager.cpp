// =====================================================
// File: src/base/node_manager.cpp
// =====================================================

#include "node_manager.h"

NodeManager::NodeManager() {
    for (uint8_t i = 0; i < NODE_COUNT; i++) {
        nodes[i].nodeId = NODE_CONFIGS[i].nodeId;

        memcpy(
            nodes[i].mac,
            NODE_CONFIGS[i].mac,
            sizeof(nodes[i].mac));

        nodes[i].online = false;
        nodes[i].lastResponse = 0;
    }
}

void NodeManager::begin() {
    Serial.println("Node Manager");
    Serial.println("------------");

    for (uint8_t i = 0; i < NODE_COUNT; i++) {
        Serial.printf(
            "Node %u configured\n",
            nodes[i].nodeId);
    }

    Serial.println();
}

uint8_t NodeManager::getNodeCount() const {
    return NODE_COUNT;
}

NodeInfo* NodeManager::getNode(uint8_t index) {
    if (index >= NODE_COUNT) {
        return nullptr;
    }

    return &nodes[index];
}

NodeInfo* NodeManager::getNodeById(uint8_t nodeId) {
    for (uint8_t i = 0; i < NODE_COUNT; i++) {
        if (nodes[i].nodeId == nodeId) {
            return &nodes[i];
        }
    }

    return nullptr;
}

void NodeManager::markOnline(uint8_t nodeId) {
    NodeInfo* node = getNodeById(nodeId);

    if (node == nullptr) {
        return;
    }

    node->online = true;
    node->lastResponse = millis();
}

void NodeManager::markOffline(uint8_t nodeId) {
    NodeInfo* node = getNodeById(nodeId);

    if (node == nullptr) {
        return;
    }

    node->online = false;
}

