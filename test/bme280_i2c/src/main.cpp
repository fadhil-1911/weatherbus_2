/*
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

constexpr uint8_t BME280_I2C_ADDRESS = 0x76;

Adafruit_BME280 bme;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.println("PHASE 2C.3 - BME280 TEST");
    Serial.println("================================");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    Serial.printf("SDA: GPIO %u\n", SDA_PIN);
    Serial.printf("SCL: GPIO %u\n", SCL_PIN);
    Serial.println("I2C Clock: 100 kHz");
    Serial.println();

    Serial.printf(
        "Initializing BME280 at 0x%02X...\n",
        BME280_I2C_ADDRESS);

    if (!bme.begin(BME280_I2C_ADDRESS, &Wire)) {
        Serial.println("ERROR: BME280 initialization failed.");
        Serial.println("Check address, wiring, and power.");

        while (true) {
            delay(1000);
        }
    }

    Serial.println("BME280 initialized successfully.");
    Serial.println();
}

void loop() {
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 100.0F;
    float humidity = bme.readHumidity();

    Serial.printf(
        "Temperature : %.2f C\n",
        temperature);

    Serial.printf(
        "Pressure    : %.2f hPa\n",
        pressure);

    Serial.printf(
        "Humidity    : %.2f %%\n",
        humidity);

    Serial.println();

    delay(2000);
} */





/* 
//read the raw pressure value from the BME280 sensor over I2C
   and print it to the serial monitor 
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

constexpr uint8_t SHT41_ADDRESS = 0x44;
constexpr uint8_t BME280_ADDRESS = 0x76;

// BME280 registers
constexpr uint8_t REG_CHIP_ID = 0xD0;
constexpr uint8_t REG_RESET = 0xE0;
constexpr uint8_t REG_CTRL_HUM = 0xF2;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_CONFIG = 0xF5;
constexpr uint8_t REG_PRESS_MSB = 0xF7;

// Expected BME280 chip ID
constexpr uint8_t BME280_CHIP_ID = 0x60;

uint8_t readRegister(uint8_t address, uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0) {
        return 0xFF;
    }

    if (Wire.requestFrom(address, static_cast<uint8_t>(1)) != 1) {
        return 0xFF;
    }

    return Wire.read();
}

void writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

bool detectBME280() {
    uint8_t chipId = readRegister(BME280_ADDRESS, REG_CHIP_ID);

    Serial.printf("BME280 Chip ID: 0x%02X\n", chipId);

    if (chipId == BME280_CHIP_ID) {
        Serial.println("BME280 detected successfully.");
        return true;
    }

    Serial.println("ERROR: BME280 chip ID mismatch.");
    return false;
}

void configureBME280() {
    // Humidity oversampling x1
    writeRegister(BME280_ADDRESS, REG_CTRL_HUM, 0x01);

    // Temperature x1, Pressure x1, Normal mode
    writeRegister(BME280_ADDRESS, REG_CTRL_MEAS, 0x27);

    // Standby 1000 ms, filter off
    writeRegister(BME280_ADDRESS, REG_CONFIG, 0xA0);

    delay(10);
}

bool readRawPressure(uint32_t& pressureRaw) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(REG_PRESS_MSB);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(BME280_ADDRESS, static_cast<uint8_t>(3)) != 3) {
        return false;
    }

    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t xlsb = Wire.read();

    pressureRaw =
        (static_cast<uint32_t>(msb) << 12) |
        (static_cast<uint32_t>(lsb) << 4) |
        (static_cast<uint32_t>(xlsb) >> 4);

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.println("PHASE 2C.2 - BME280 READING TEST");
    Serial.println("================================");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    Serial.printf("SDA: GPIO %u\n", SDA_PIN);
    Serial.printf("SCL: GPIO %u\n", SCL_PIN);
    Serial.println("I2C Clock: 100 kHz");
    Serial.println();

    Serial.println("Checking SHT41...");
    uint8_t sht41 = readRegister(SHT41_ADDRESS, 0x00);

    if (sht41 == 0xFF) {
        Serial.println("SHT41 communication check unavailable.");
    } else {
        Serial.println("SHT41 is present on I2C bus.");
    }

    Serial.println();

    if (!detectBME280()) {
        Serial.println("SYSTEM HALTED");
        while (true) {
            delay(1000);
        }
    }

    configureBME280();

    Serial.println("BME280 configured.");
    Serial.println();
}

void loop() {
    uint32_t pressureRaw = 0;

    if (readRawPressure(pressureRaw)) {
        Serial.printf(
            "BME280 Pressure Raw: %lu\n",
            static_cast<unsigned long>(pressureRaw));
    } else {
        Serial.println("ERROR: Failed to read BME280 pressure.");
    }

    delay(2000);
}  */








/* 
// WeatherBus V1.0
//  PHASE 2C.1 - BME280 I2C TEST
//  
//  This code is designed to test the I2C communication with the BME280 sensor.
//  It scans the I2C bus for connected devices and identifies the BME280 sensor.
 
#include <Arduino.h>
#include <Wire.h>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

void scanI2C() {
    Serial.println();
    Serial.println("========== I2C SCANNER ==========");

    uint8_t found = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf("I2C device found at 0x%02X", address);

            if (address == 0x44) {
                Serial.print("  <- SHT41");
            } else if (address == 0x76) {
                Serial.print("  <- BME280");
            } else if (address == 0x77) {
                Serial.print("  <- BME280");
            }

            Serial.println();
            found++;
        }
    }

    if (found == 0) {
        Serial.println("No I2C devices found.");
    }

    Serial.printf("Total devices: %u\n", found);
    Serial.println("=================================");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("WeatherBus V1.0");
    Serial.println("PHASE 2C.1 - BME280 I2C TEST");
    Serial.println("================================");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    Serial.printf("SDA: GPIO %u\n", SDA_PIN);
    Serial.printf("SCL: GPIO %u\n", SCL_PIN);
    Serial.println("I2C Clock: 100 kHz");

    scanI2C();
}

void loop() {
    delay(3000);
    scanI2C();
} */


