// =====================================================
// File: src/services/web/WebDashboardService.h
// =====================================================

#pragma once

#include <Arduino.h>
#include <WebServer.h>

class WebDashboardService {
public:
    bool begin();
    void update();

    void setLocalData(
        float temperature,
        float humidity,
        uint8_t flags,
        bool valid);

    void setNodeData(
        uint8_t nodeId,
        float temperature,
        float humidity,
        float pressure,
        uint8_t flags);

private:
    WebServer server_{80};
    bool started_ = false;

    struct SensorData {
        float temperature = 0.0f;
        float humidity = 0.0f;
        float pressure = 0.0f;
        uint8_t flags = 0;
        bool valid = false;
        uint32_t lastUpdate = 0;
    };

    SensorData localData_;
    SensorData node1Data_;

    void handleRoot();
    void handleLocalApi();
    void handleNodesApi();
    void handleNotFound();
};
