// =====================================================
// File: src/base/RTCManager.h
// =====================================================

#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <RTClib.h>

class RTCManager {
  public:
    bool begin();
    bool update();
    bool isOK();
    DateTime now();
    bool setDateTime(const DateTime& dateTime);

  private:
    RTC_DS3231 _rtc;
    DateTime _now;
    bool _status = false;
};

extern RTCManager rtc;

#endif