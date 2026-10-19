#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

// ESP32-C3 Base Station I2C pins
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;

RTC_DS3231 rtc;

void scanI2C() {
    Serial.println();
    Serial.println("=== I2C SCAN ===");

    uint8_t deviceCount = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf(
                "I2C device found at 0x%02X\n",
                address);
            deviceCount++;
        }
    }

    if (deviceCount == 0) {
        Serial.println("FAIL: No I2C device detected");
    } else {
        Serial.printf(
            "I2C scan complete: %u device(s)\n",
            deviceCount);
    }
}

void setRTCTime() {
    Serial.println("Setting RTC time from compilation time...");

    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    Serial.println("RTC time set.");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("==============================");
    Serial.println("RTC DS3231 MODULE TEST");
    Serial.println("==============================");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);
    delay(100);

    scanI2C();

    Serial.println();
    Serial.println("=== RTC INITIALIZATION ===");

    if (!rtc.begin()) {
        Serial.println("FAIL: DS3231 not detected");
        Serial.println("Check power, SDA, SCL and wiring.");
        return;
    }

    Serial.println("PASS: DS3231 detected");

    if (rtc.lostPower()) {
        Serial.println("WARNING: RTC reports power loss.");
    } else {
        Serial.println("RTC does not report power loss.");
    }

    // Set RTC using compilation date and time
    setRTCTime();

    // Verify the newly set time
    DateTime now = rtc.now();

    Serial.printf(
        "Initial RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second());

    Serial.println();
    Serial.println("=== RTC REPEATED READ TEST ===");
}

void loop() {
    static uint32_t testCount = 0;

    testCount++;

    Serial.printf("\nTest #%lu\n", (unsigned long)testCount);

    DateTime now = rtc.now();

    Serial.printf(
        "RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second());

    if (now.year() < 2020 || now.year() > 2100) {
        Serial.println("FAIL: RTC time is outside expected range");
    } else {
        Serial.println("PASS: RTC time is within expected range");
    }

    delay(1000);
}