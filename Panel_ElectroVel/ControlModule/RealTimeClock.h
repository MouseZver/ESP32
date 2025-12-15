#ifndef REALTIMECLOCK_H
#define REALTIMECLOCK_H

#include <RTClib.h>

class RealTimeClock {
public:
    RealTimeClock();

    void begin();
    void update();

    String getFormattedTime(); // Формат: "Mon 1, 25 Dec 2024"

private:
    RTC_DS3231 _rtc;

    unsigned long _lastUpdate = 0;
    uint32_t _updateInterval = 1000; // 1 секунда

    String _formattedTime;

    // Вспомогательные данные
    static const char* _daysOfWeek[];
    static const char* _months[];
};

#endif