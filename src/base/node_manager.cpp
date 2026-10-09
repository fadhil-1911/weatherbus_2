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

/*
    // =====================================================
// File: src/base/node_manager.cpp
// =====================================================

#include "node_manager.h"

NodeManager::NodeManager() {

    // -------------------------------------------------
    // Node 1
    // -------------------------------------------------

    nodes[0].nodeId = 1;
    uint8_t node1Mac[6] = {
        0x9C, 0xCC, 0x01,
        0x7D, 0x14, 0x80};

    memcpy(nodes[0].mac, node1Mac, 6);

    nodes[0].online = false;
    nodes[0].lastResponse = 0;

    // -------------------------------------------------
    // Node 2
    // -------------------------------------------------
    
    //nodes[1].nodeId = 1;
    //uint8_t node2Mac[6] = {
    //    0x0C, 0x4E, 0xA0,
    //    0x4D, 0x7C, 0x44};

    //memcpy(nodes[1].mac, node2Mac, 6);

    //nodes[1].online = false;
    //nodes[1].lastResponse = 0; 
}

void NodeManager::begin() {
    Serial.println("Node Manager");
    Serial.println("------------");

    for (uint8_t i = 0; i < MAX_NODES; i++) {
        Serial.printf(
            "Node %u configured\n",
            nodes[i].nodeId);
    }
    Serial.println();
}

uint8_t NodeManager::getNodeCount() const {
    return MAX_NODES;
}

NodeInfo* NodeManager::getNode(uint8_t index) {
    if (index >= MAX_NODES) {
        return nullptr;
    }
    return &nodes[index];
}

NodeInfo* NodeManager::getNodeById(uint8_t nodeId) {
    for (uint8_t i = 0; i < MAX_NODES; i++) {
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
} */