// =====================================================
// File: src/config/node_config.h
// =====================================================

#pragma once

#include <Arduino.h>

struct NodeConfig
{
    uint8_t nodeId;
    uint8_t mac[6];
};

constexpr NodeConfig NODE_CONFIGS[] = {
    {1, {0x9C, 0xCC, 0x01, 0x7D, 0x14, 0x80}}, // Node 1
    //{2, {0x9C, 0xCC, 0x01, 0x7D, 0x14, 0x81}}, // Node 2
};

constexpr uint8_t NODE_COUNT =
    sizeof(NODE_CONFIGS) / sizeof(NODE_CONFIGS[0]);