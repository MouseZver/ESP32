#include "RealTimeClock.h"
#include <Wire.h>

// Статические данные
const char* RealTimeClock::_daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* RealTimeClock::_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

RealTimeClock::RealTimeClock() {
    // Конструктор
}

void RealTimeClock::begin() {
    Wire.begin();
    if (!_rtc.begin()) {
        Serial.println("Не могу найти RTC");
        while (true); // Остановка
    }

    // Установить время при первом запуске (можно закомментировать после первого запуска)
    //_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void RealTimeClock::update() {
    if (millis() - _lastUpdate >= _updateInterval) {
        DateTime now = _rtc.now();

        String dayOfWeekStr = _daysOfWeek[now.dayOfTheWeek()];
        int dayOfWeekNum = now.dayOfTheWeek();
        int day = now.day();
        String monthStr = _months[now.month() - 1]; // месяцы с 1
        int year = now.year();

        _formattedTime = dayOfWeekStr + " " + String(dayOfWeekNum) + ", " +
                         String(day) + " " + monthStr + " " + String(year);

        _lastUpdate = millis();
    }
}

String RealTimeClock::getFormattedTime() {
    return _formattedTime;
}