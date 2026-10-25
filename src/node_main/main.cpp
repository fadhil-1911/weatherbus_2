//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    WeatherBus
//                   Version: 1.0
//             Last Updated: 2026-10-20
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
  Module    : Sensor Node - Main Application
  Transport : ESP-NOW
  Phase   : 
*/

#include <Arduino.h>
#include <Wire.h>
#include "../config/node/node_settings.h"
#include "../drivers/BME280Source.h"
#include "../drivers/SHT41Source.h"
#include "../drivers/ESPNowTransport.h"
#include "../services/node/NodeCommunicationService.h"

#include "../common/weatherbus_protocol.h"

#include <cstring>

using namespace WeatherBus;


// =====================================================
// SENSOR DRIVER OBJECT
// =====================================================

SHT41Source sht41Source;
BME280Source bme280Source(BME280_I2C_ADDRESS);
ESPNowTransport transport;

NodeCommunicationService nodeCommunicationService(
    transport,
    bme280Source,
    sht41Source);


bool setupI2C() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_CLOCK);

    delay(10);

    Serial.println("I2C bus ready");
    return true;
}

// =====================================================
// Setup
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!setupI2C()) {
        Serial.println("I2C initialization failed");

        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.printf("NODE ID: %u\n", NODE_ID);
    Serial.println("================================");

    Serial.println();
    Serial.println("Sensor Configuration:");

    Serial.printf(
        "SHT41  : %s\n",
        ENABLE_SHT41 ? "ENABLED" : "DISABLED");

    Serial.printf(
        "BME280 : %s\n",
        ENABLE_BME280 ? "ENABLED" : "DISABLED");

    Serial.println();

    if (ENABLE_SHT41) {
        if (!sht41Source.begin()) {
            Serial.println("SHT41 FAILED");
            Serial.println(
                "SHT41 unavailable - continuing without temperature/humidity");
        }
    }

    if (ENABLE_BME280) {
        if (!bme280Source.begin()) {
            Serial.println("BME280 FAILED");
            Serial.println("BME280 unavailable - continuing without pressure");
        }
    }

    TransportAddress baseAddress{};
    baseAddress.length = sizeof(baseMac);

    memcpy(
        baseAddress.data,
        baseMac,
        baseAddress.length);

    if (!nodeCommunicationService.begin(baseAddress)) {
        Serial.println("SYSTEM HALTED");

        while (true) {
            delay(1000);
        }
    }

    Serial.println("ESP-NOW ready");
    Serial.println("Node ready");
}

// =====================================================
// Loop
// =====================================================

void loop() {
    // No polling required.
    //
    // Node responds only when a valid
    // DATA_REQUEST is received.
    nodeCommunicationService.update();
}
