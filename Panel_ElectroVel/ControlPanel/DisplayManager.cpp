#include "DisplayManager.h"

DisplayManager::DisplayManager(TFT_eSPI& display) : tft(display) {}

void DisplayManager::begin() {
    for (int i = 0; i < 6; i++) changed[i] = true;
}

void DisplayManager::update() {
    if (blocked) return; // Ничего не делаем, если заблокировано

    // === Автоматическое обновление времени ===
    if (unixTimeSet) {
        uint32_t nowUnix = baseUnixTime + (millis() - baseMillis) / 1000;

        // Обновляем только если секунда изменилась
        int currentSecond = (nowUnix % 86400) % 60; // секунда в текущих сутках
        if (currentSecond != lastSecond) {
            lastSecond = currentSecond;

            int year, month, day, hour, minute, second, weekday;
            parseUnixTime(nowUnix, year, month, day, hour, minute, second, weekday);
            setTimeStr(hour, minute, second);
            setDateStr(year, month, day, weekday);
        }
    }

    // === Проверяем мигание мотора ===
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

    // === Проверяем мигание температуры ===
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

    // === Рендерим только изменённые элементы ===
    if (changed[0]) drawTime(); changed[0] = false;
    if (changed[1]) drawDate(); changed[1] = false;
    if (changed[2]) drawTemperature(); changed[2] = false;
    if (changed[3]) drawMotorStatus(); changed[3] = false;
    if (changed[4]) drawLampStatus(); changed[4] = false;
    if (changed[5]) drawBackLampStatus(); changed[5] = false;
}

bool DisplayManager::isDaylightSavingTime(int year, int month, int day, int weekday) {
    // Пример: европейское летнее время — с последнего воскресенья марта по последнее воскресенье октября
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

    // Применяем часовой пояс (UTC+3)
    unixTime += TIME_ZONE_OFFSET * SECONDS_PER_HOUR;

    // Если нужно — учитываем летнее время (пример: +1 час в Европе с последней воскресенье марта по последнее воскресенье октября)
    if (useDST) {
        bool isDST = isDaylightSavingTime(year, month, day, weekday);
        if (isDST) {
            unixTime += SECONDS_PER_HOUR; // +1 час на летнее время
        }
    }

    uint32_t days = unixTime / SECONDS_PER_DAY;
    uint32_t remainder = unixTime % SECONDS_PER_DAY;

    hour = remainder / SECONDS_PER_HOUR;
    remainder %= SECONDS_PER_HOUR;
    minute = remainder / SECONDS_PER_MINUTE;
    second = remainder % SECONDS_PER_MINUTE;

    // Коррекция дней после сдвига времени
    remainder = 0;
    unixTime = days * SECONDS_PER_DAY + remainder;

    // Вычисление даты
    int a = days + 719468; // сдвиг к 1 марта 0000
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

    // День недели: 1970-01-01 — четверг (weekday = 4)
    int dayOfWeek = (days + 4) % 7; // 0 = Thursday, 3 = Sunday
    weekday = (dayOfWeek == 0) ? 7 : dayOfWeek; // 1 = Monday ... 7 = Sunday
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
        dayNames[weekday % 7 == 0 ? 6 : weekday % 7 - 1], // день недели (Mon)
        weekday, // номер дня недели (1-7)
        day,     // день месяца
        monthNames[month - 1], // месяц
        year);   // год

    if (dateStr != buffer) {
        dateStr = buffer;
        changed[1] = true;
    }
}

// === Установщики ===
/*void DisplayManager::setTime(const String& time) {
    if (time != timeStr) {
        timeStr = time;
        changed[0] = true;
    }
}*/
void DisplayManager::setTime(uint32_t unixTime) {
    baseUnixTime = unixTime;
    baseMillis = millis();
    unixTimeSet = true;

    // Разбор начального времени
    int year, month, day, hour, minute, second, weekday;
    parseUnixTime(baseUnixTime, year, month, day, hour, minute, second, weekday);
    // Установка времени
    setTimeStr(hour, minute, second);
    // Установка даты
    setDateStr(year, month, day, weekday);
}
/*void DisplayManager::setDate(const String& date) {
    if (date != dateStr) {
        dateStr = date;
        changed[1] = true;
    }
}*/
void DisplayManager::setTemperature(float temp) {
    if (abs(temp - temperature) > 0.1) {
        temperature = temp;
        changed[2] = true;
    }
}
void DisplayManager::setMotorStatus(bool status) {
    if (status != motorStatus) {
        // === Статус изменился ===
        motorStatus = status;
        motorStartMillis = millis(); // Сброс таймера мигания
        motorBlinkEnabled = true;    // Включаем мигание
        changed[3] = true;
    } else {
        // === Статус не изменился — переключаем мигание ===
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

// === Функции отрисовки ===
void DisplayManager::drawTime() {
    if (blocked) return; // Ничего не делаем, если заблокировано

    tft.setTextSize(3);
    tft.setTextColor(TFT_LIGHTGREY);

    int16_t centerY = TIME_Y;// + (LINE_HEIGHT / 2); // центрируем по высоте
    int16_t tw = tft.textWidth(timeStr.c_str());
    String currentStr = timeStr;

    clearStatusArea(120, centerY, currentStr, prevTimeStr);
    prevTimeStr = currentStr;

    tft.setCursor(120 - tw / 2, TIME_Y);
    tft.print(currentStr);
}

void DisplayManager::drawDate() {
    if (blocked) return; // Ничего не делаем, если заблокировано

    tft.setTextSize(2);
    tft.setTextColor(TFT_GREENYELLOW);

    int16_t centerY = DATE_Y;// + (LINE_HEIGHT / 2);
    int16_t tw = tft.textWidth(dateStr.c_str());
    String currentStr = dateStr;

    clearStatusArea(120, centerY, currentStr, prevDateStr);
    prevDateStr = currentStr;

    tft.setCursor(120 - tw / 2, DATE_Y);
    tft.print(currentStr);
}

void DisplayManager::drawTemperature() {
    if (blocked) return; // Ничего не делаем, если заблокировано

    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(LABEL_X, TEMP_LABEL_Y);
    tft.println("Temperature");

    tft.setCursor(LABEL_X, TEMP_LABEL_Y + 24);
    tft.println("Controller");

    int fontHeight = tft.fontHeight();
    int centerY = TEMP_LABEL_Y + (LINE_HEIGHT / 2);

    char buffer[10];
    sprintf(buffer, "%.2f", temperature);
    String currentStr = String(buffer);

    // === Очистка области предыдущего текста ===
    clearStatusArea(VALUE_X, centerY, currentStr, prevTemperatureStr);
    prevTemperatureStr = currentStr;

    // === Установка цвета в зависимости от состояния ===
    if (temperature > 40 && tempBlinkState) {
        tft.setTextColor(TFT_YELLOW, TFT_RED);
    } else if (temperature > 40) {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    } else {
        tft.setTextColor(TFT_YELLOW);
    }

    // === Центрирование текста ===
    int16_t tw = tft.textWidth(currentStr.c_str());
    tft.setCursor(VALUE_X - tw / 2, centerY);
    tft.print(currentStr);
}

void DisplayManager::drawMotorStatus() {
    if (blocked) return; // Ничего не делаем, если заблокировано

    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(LABEL_X, STATUS1_Y);
    tft.println("Power Motor");

    String statusStr = motorStatus ? "[ON]" : "[OFF]";

    // Очистка с учётом предыдущего текста
    clearStatusArea(VALUE_X, STATUS1_Y, statusStr, prevMotorStatusStr);
    prevMotorStatusStr = statusStr;

    // === Установка цвета ===
    if (motorStatus) {
        if (motorBlinkEnabled) {
            if (motorBlinkState) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK); // [ON] — зеленый текст на черном фоне
            } else {
                tft.setTextColor(TFT_BLACK, TFT_GREEN); // [ON] — черный текст на зеленом фоне
            }
        } else {
            tft.setTextColor(TFT_BLACK, TFT_GREEN); // стабильный [ON]
        }
    } else {
        if (motorBlinkEnabled) {
            if (motorBlinkState) {
                tft.setTextColor(TFT_BLACK, TFT_GREEN); // [OFF] фон
            } else {
                tft.setTextColor(TFT_YELLOW, TFT_BLACK); // [OFF] желтый текст
            }
        } else {
            tft.setTextColor(TFT_YELLOW, TFT_BLACK); // стабильный [OFF]
        }
    }

    // === Вывод текста ===
    int16_t tw = tft.textWidth(statusStr.c_str());
    tft.setCursor(VALUE_X - tw / 2, STATUS1_Y);
    tft.print(statusStr);
}

void DisplayManager::drawLampStatus() {
    if (blocked) return; // Ничего не делаем, если заблокировано

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
    if (blocked) return; // Ничего не делаем, если заблокировано
    
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
    // Опционально: можно вызвать update() или перерисовать экран после разблокировки,
    // если требуется немедленное обновление. Пока просто устанавливаем флаг.
}

// === Вспомогательный метод ===
void DisplayManager::clearStatusArea(int x, int y, const String& currentStr, const String& prevStr) {
    // Вычисляем ширину текста
    int16_t prevWidth = tft.textWidth(prevStr.c_str());
    int16_t currWidth = tft.textWidth(currentStr.c_str());
    int width = max(prevWidth, currWidth);

    // Добавляем небольшой запас, чтобы избежать "хвостов"
    const int padding = 6;
    const int textHeight = 22;//20

    // Так как `x` — это центр, то смещаемся влево на половину ширины
    int xStart = x - (width / 2) - padding / 2;

    // Очищаем область с запасом
    tft.fillRect(xStart, y, width + padding, textHeight, TFT_BLACK);
}