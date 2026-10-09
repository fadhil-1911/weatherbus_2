// =====================================================
// File: src/drivers/RTCClock.h
// =====================================================

#pragma once

#include "../interfaces/IClock.h"

class RTCClock : public IClock {
  public:
    bool begin() override;
    bool update() override;
    bool isOK() const override;

    DateTime now() const override;

    bool setDateTime(
        const DateTime& dateTime) override;

  private:
    RTC_DS3231 _rtc;
    DateTime _now;
    bool _status = false;
};