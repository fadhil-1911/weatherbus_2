// =====================================================
// File: src/interfaces/IClock.h
// =====================================================

#pragma once

#include <RTClib.h>

class IClock {
  public:
    virtual ~IClock() = default;

    virtual bool begin() = 0;
    virtual bool update() = 0;
    virtual bool isOK() const = 0;

    virtual DateTime now() const = 0;

    virtual bool setDateTime(
        const DateTime& dateTime) = 0;
};