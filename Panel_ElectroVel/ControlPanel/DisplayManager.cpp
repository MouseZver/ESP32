#include "DisplayManager.h"

DisplayManager::DisplayManager(TFT_eSPI& display) : tft(display) {}

void DisplayManager::begin() {
    for (int i = 0; i < 6; i++) changed[i] = true;
}

void DisplayManager::update() {
    if (blocked) return;

    if (unixTimeSet) {
        uint32_t nowUnix = baseUnixTime + (millis() - baseMillis) / 1000;
        int currentSecond = (nowUnix % 86400) % 60;
        if (currentSecond != lastSecond) {
            lastSecond = currentSecond;
            int year, month, day, hour, minute, second, weekday;
            parseUnixTime(nowUnix, year, month, day, hour, minute, second, weekday);
            setTimeStr(hour, minute, second);
            setDateStr(year, month, day, weekday);
        }
    }

    if (motorBlinkEnabled) {
        if ((millis() - motorStartMillis) / 500 % 2 == 0) {
            if (!motorBlinkState) {
                changed[3] = true;
                motorBlinkState = true;
            }
        } else {
            if (motorBlinkState) {
                changed[3] = true;
                motorBlinkState = false;
            }
        }
    }

    if (temperature > 40) {
        if ((millis() - tempBlinkMillis) / 1000 % 2 == 0) {
            if (!tempBlinkState) {
                changed[2] = true;
                tempBlinkState = true;
            }
        } else {
            if (tempBlinkState) {
                changed[2] = true;
                tempBlinkState = false;
            }
        }
    } else {
        if (tempBlinkState) {
            changed[2] = true;
            tempBlinkState = false;
        }
    }

    if (changed[0]) drawTime(); changed[0] = false;
    if (changed[1]) drawDate(); changed[1] = false;
    if (changed[2]) drawTemperature(); changed[2] = false;
    if (changed[3]) drawMotorStatus(); changed[3] = false;
    if (changed[4]) drawLampStatus(); changed[4] = false;
    if (changed[5]) drawBackLampStatus(); changed[5] = false;
}

bool DisplayManager::isDaylightSavingTime(int year, int month, int day, int weekday) {
    if (month < 3 || month > 10) return false;
    if (month > 3 && month < 10) return true;
    int lastSundayMarch = 31;
    while (true) {
        int testWeekday = (weekday - (31 - lastSundayMarch) + 7) % 7;
        if (testWeekday == 0) break;
        lastSundayMarch--;
    }
    int lastSundayOctober = 31;
    while (true) {
        int testWeekday = (weekday - (31 - lastSundayOctober) + 7) % 7;
        if (testWeekday == 0) break;
        lastSundayOctober--;
    }
    if (month == 3 && day > lastSundayMarch) return true;
    if (month == 10 && day <= lastSundayOctober) return true;
    return false;
}

void DisplayManager::parseUnixTime(uint32_t unixTime, int& year, int& month, int& day, int& hour, int& minute, int& second, int& weekday) {
    const int SECONDS_PER_MINUTE = 60;
    const int SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
    const int SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;

    unixTime += TIME_ZONE_OFFSET * SECONDS_PER_HOUR;
    if (useDST) {
        bool isDST = isDaylightSavingTime(year, month, day, weekday);
        if (isDST) {
            unixTime += SECONDS_PER_HOUR;
        }
    }

    uint32_t days = unixTime / SECONDS_PER_DAY;
    uint32_t remainder = unixTime % SECONDS_PER_DAY;
    hour = remainder / SECONDS_PER_HOUR;
    remainder %= SECONDS_PER_HOUR;
    minute = remainder / SECONDS_PER_MINUTE;
    second = remainder % SECONDS_PER_MINUTE;

    remainder = 0;
    unixTime = days * SECONDS_PER_DAY + remainder;

    int a = days + 719468;
    int b = (a * 4 + 3) / 146097;
    int c = a - (b * 146097) / 4;
    int d = (c * 4 + 3) / 1461;
    int e = c - (1461 * d) / 4;
    int m = (5 * e + 2) / 153;
    day = e - (153 * m + 2) / 5 + 1;
    month = m + 3;
    if (month > 12) {
        month -= 12;
        year = d + 1;
    } else {
        year = d;
    }

    int dayOfWeek = (days + 4) % 7;
    weekday = (dayOfWeek == 0) ? 7 : dayOfWeek;
}

void DisplayManager::setTimeStr(int hour, int minute, int second) {
    char buffer[9];
    sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
    if (timeStr != buffer) {
        timeStr = buffer;
        changed[0] = true;
    }
}

void DisplayManager::setDateStr(int year, int month, int day, int weekday) {
    char buffer[30];
    sprintf(buffer, "%s %d, %d %s %d",
        dayNames[weekday % 7 == 0 ? 6 : weekday % 7 - 1],
        weekday,
        day,
        monthNames[month - 1],
        year);
    if (dateStr != buffer) {
        dateStr = buffer;
        changed[1] = true;
    }
}

void DisplayManager::setTime(uint32_t unixTime) {
    baseUnixTime = unixTime;
    baseMillis = millis();
    unixTimeSet = true;
    int year, month, day, hour, minute, second, weekday;
    parseUnixTime(baseUnixTime, year, month, day, hour, minute, second, weekday);
    setTimeStr(hour, minute, second);
    setDateStr(year, month, day, weekday);
}

void DisplayManager::setTemperature(float temp) {
    if (abs(temp - temperature) > 0.1) {
        temperature = temp;
        changed[2] = true;
    }
}

void DisplayManager::setMotorStatus(bool status) {
    if (status != motorStatus) {
        motorStatus = status;
        motorStartMillis = millis();
        motorBlinkEnabled = true;
        changed[3] = true;
    } else {
        motorBlinkEnabled = !motorBlinkEnabled;
        changed[3] = true;
    }
}

void DisplayManager::setLampStatus(bool status) {
    if (status != lampStatus) {
        lampStatus = status;
        changed[4] = true;
    }
}

void DisplayManager::setBackLampStatus(bool status) {
    if (status != backLampStatus) {
        backLampStatus = status;
        changed[5] = true;
    }
}

void DisplayManager::drawTime() {
    if (blocked) return;
    tft.setTextSize(3);
    tft.setTextColor(TFT_LIGHTGREY);
    int16_t centerY = TIME_Y;
    int16_t tw = tft.textWidth(timeStr.c_str());
    String currentStr = timeStr;
    clearStatusArea(120, centerY, currentStr, prevTimeStr);
    prevTimeStr = currentStr;
    tft.setCursor(120 - tw / 2, TIME_Y);
    tft.print(currentStr);
}

void DisplayManager::drawDate() {
    if (blocked) return;
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREENYELLOW);
    int16_t centerY = DATE_Y;
    int16_t tw = tft.textWidth(dateStr.c_str());
    String currentStr = dateStr;
    clearStatusArea(120, centerY, currentStr, prevDateStr);
    prevDateStr = currentStr;
    tft.setCursor(120 - tw / 2, DATE_Y);
    tft.print(currentStr);
}

void DisplayManager::drawTemperature() {
    if (blocked) return;
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(LABEL_X, TEMP_LABEL_Y);
    tft.println("Temperature");
    tft.setCursor(LABEL_X, TEMP_LABEL_Y + 24);
    tft.println("Controller");
    
    // УДАЛЕНО: int fontHeight = tft.fontHeight(); (переменная не использовалась)
    
    int centerY = TEMP_LABEL_Y + (LINE_HEIGHT / 2);
    char buffer[10];
    sprintf(buffer, "%.2f", temperature);
    String currentStr = String(buffer);
    
    clearStatusArea(VALUE_X, centerY, currentStr, prevTemperatureStr);
    prevTemperatureStr = currentStr;
    
    if (temperature > 40 && tempBlinkState) {
        tft.setTextColor(TFT_YELLOW, TFT_RED);
    } else if (temperature > 40) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_YELLOW);
    }
    
    int16_t tw = tft.textWidth(currentStr.c_str());
    tft.setCursor(VALUE_X - tw / 2, centerY);
    tft.print(currentStr);
}

void DisplayManager::drawMotorStatus() {
    if (blocked) return;
    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(LABEL_X, STATUS1_Y);
    tft.println("Power Motor");
    String statusStr = motorStatus ? "[ON]" : "[OFF]";
    
    clearStatusArea(VALUE_X, STATUS1_Y, statusStr, prevMotorStatusStr);
    prevMotorStatusStr = statusStr;
    
    if (motorStatus) {
        if (motorBlinkEnabled) {
            if (motorBlinkState) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
            } else {
                tft.setTextColor(TFT_BLACK, TFT_GREEN);
            }
        } else {
            tft.setTextColor(TFT_BLACK, TFT_GREEN);
        }
    } else {
        if (motorBlinkEnabled) {
            if (motorBlinkState) {
                tft.setTextColor(TFT_BLACK, TFT_GREEN);
            } else {
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            }
        } else {
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        }
    }
    
    int16_t tw = tft.textWidth(statusStr.c_str());
    tft.setCursor(VALUE_X - tw / 2, STATUS1_Y);
    tft.print(statusStr);
}

void DisplayManager::drawLampStatus() {
    if (blocked) return;
    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(LABEL_X, STATUS2_Y);
    tft.println("Lamp");
    String statusStr = lampStatus ? "[ON]" : "[OFF]";
    int16_t tw = tft.textWidth(statusStr.c_str());
    
    clearStatusArea(VALUE_X, STATUS2_Y, statusStr, prevLampStatusStr);
    prevLampStatusStr = statusStr;
    
    if (lampStatus) {
        tft.setTextColor(TFT_BLACK, TFT_GREEN);
    } else {
        tft.setTextColor(TFT_YELLOW);
    }
    tft.setCursor(VALUE_X - tw / 2, STATUS2_Y);
    tft.print(statusStr);
}

void DisplayManager::drawBackLampStatus() {
    if (blocked) return;
    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(LABEL_X, STATUS3_Y);
    tft.println("Back Lamp");
    String statusStr = backLampStatus ? "[ON]" : "[OFF]";
    int16_t tw = tft.textWidth(statusStr.c_str());
    
    clearStatusArea(VALUE_X, STATUS3_Y, statusStr, prevBackLampStatusStr);
    prevBackLampStatusStr = statusStr;
    
    if (backLampStatus) {
        tft.setTextColor(TFT_BLACK, TFT_GREEN);
    } else {
        tft.setTextColor(TFT_YELLOW);
    }
    tft.setCursor(VALUE_X - tw / 2, STATUS3_Y);
    tft.print(statusStr);
}

void DisplayManager::block(bool state) {
    blocked = state;
}

void DisplayManager::clearStatusArea(int x, int y, const String& currentStr, const String& prevStr) {
    int16_t prevWidth = tft.textWidth(prevStr.c_str());
    int16_t currWidth = tft.textWidth(currentStr.c_str());
    int width = max(prevWidth, currWidth);
    const int padding = 6;
    const int textHeight = 22;
    int xStart = x - (width / 2) - padding / 2;
    tft.fillRect(xStart, y, width + padding, textHeight, TFT_BLACK);
}