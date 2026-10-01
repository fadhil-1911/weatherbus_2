// =====================================================
// File: src/base/RTCManager.cpp
// =====================================================

#include "RTCManager.h"
#include <Arduino.h>
#include <Wire.h>

//==========================================================
// Global RTC Object (externally accessible/extern)
//==========================================================

RTCManager rtc;

//==========================================================
// Begin
//==========================================================

bool RTCManager::begin() {
    Wire.begin(8, 9);
    Wire.setClock(100000);
    if (!_rtc.begin()) {
        _status = false;
        return false;
    }
    _status = true;
    return true;
}

//==========================================================
// Update
//==========================================================

bool RTCManager::update() {
    // Read DS3231
    DateTime currentTime = _rtc.now();

    // Basic RTC date validation
    if (
        currentTime.year() < 2020 ||
        currentTime.year() > 2100) {
        _status = false;
        return false;
    }

    // Store valid RTC data
    _now = currentTime;
    _status = true;
    return true;
} 

//==========================================================
// Status
//==========================================================

bool RTCManager::isOK() {
    return _status;
}

//==========================================================
// Current DateTime
//==========================================================

DateTime RTCManager::now() {
    return _now;
}

//==========================================================
// Set Date & Time
//==========================================================

bool RTCManager::setDateTime(const DateTime& dateTime) {
    _rtc.adjust(dateTime);
    return update();
}