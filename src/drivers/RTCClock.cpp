// =====================================================
// File: src/drivers/RTCClock.cpp
// =====================================================

#include "RTCClock.h"

#include <Arduino.h>
#include <Wire.h>

bool RTCClock::begin() {
    Wire.begin(8, 9);
    Wire.setClock(100000);

    if (!_rtc.begin()) {
        _status = false;
        return false;
    }

    _status = true;
    return true;
}

bool RTCClock::update() {
    DateTime currentTime = _rtc.now();

    if (currentTime.year() < 2020 ||
        currentTime.year() > 2100) {
        _status = false;
        return false;
    }

    _now = currentTime;
    _status = true;

    return true;
}

bool RTCClock::isOK() const {
    return _status;
}

DateTime RTCClock::now() const {
    return _now;
}

bool RTCClock::setDateTime(
    const DateTime& dateTime) {
    _rtc.adjust(dateTime);

    return update();
}