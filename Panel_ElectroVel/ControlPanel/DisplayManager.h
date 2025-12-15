#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

class DisplayManager {
public:
    DisplayManager(TFT_eSPI& display);

    void begin();
    void update();

    // === Методы для обновления состояний ===
    void setTime(uint32_t unixTime);
    //void setDate(const String& date);
    void setTemperature(float temp);
    void setMotorStatus(bool status);
    inline bool getMotorBlinkStatus() const { return motorBlinkEnabled; }
    void setLampStatus(bool status);
    void setBackLampStatus(bool status);
    void block(bool state); // Функция для установки/снятия блокировки

private:
    TFT_eSPI& tft;

    bool blocked = false; // Флаг блокировки отрисовки

    // time 
    static const int TIME_ZONE_OFFSET = 0; // Москва — UTC+3
    bool useDST = false; // true — если нужно учитывать летнее время

    uint32_t baseUnixTime = 0;        // Базовое UNIX-время
    uint32_t baseMillis = 0;           // Время установки (millis())
    bool unixTimeSet = false;          // Флаг: было ли установлено время
    int lastSecond = -1;               // Последняя секунда для обновления

    const char* dayNames[7] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    const char* monthNames[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    bool isDaylightSavingTime(int year, int month, int day, int weekday);
    void parseUnixTime(uint32_t unixTime, int& year, int& month, int& day, int& hour, int& minute, int& second, int& weekday);
    void setTimeStr(int hour, int minute, int second);
    void setDateStr(int year, int month, int day, int weekday);

    // === Новое поле для отслеживания предыдущего текста ===
    String prevMotorStatusStr = "[OFF]";
    String prevLampStatusStr = "[OFF]";
    String prevBackLampStatusStr = "[OFF]";
    String prevTemperatureStr = "00.00";
    String prevTimeStr = "00:00:00";
    String prevDateStr = "Mon 1, 25 Dec 2025";

    // === Состояния ===
    String timeStr = "00:00:00";
    String dateStr = "Mon 1, 25 Dec 2025";
    float temperature = 0.00;
    bool motorStatus = false;
    bool lampStatus = false;
    bool backLampStatus = false;

    // === Переменные для мигания ===
    unsigned long motorStartMillis = 0;
    unsigned long tempBlinkMillis = 0;
    bool motorBlinkState = false;
    bool tempBlinkState = false;
    bool motorBlinkEnabled = false; // true - мигание включено, false - выключено

    // === Метки изменений ===
    bool changed[6]; // 0=time, 1=date, 2=temp, 3=motor, 4=lamp, 5=backlamp

    // === Позиции на экране ===
    static const int TIME_Y = 10;
    static const int DATE_Y = 40;
    static const int LABEL_X = 10;
    static const int VALUE_X = 195;
    static const int TEMP_LABEL_Y = 60;
    static const int STATUS1_Y = 110;
    static const int STATUS2_Y = 140;
    static const int STATUS3_Y = 170;
    static const int LINE_HEIGHT = 24;

    // === Приватные методы отрисовки ===
    void drawTime();
    void drawDate();
    void drawTemperature();
    void drawMotorStatus();
    void drawLampStatus();
    void drawBackLampStatus();

    // === Вспомогательные методы ===
    void clearStatusArea(int x, int y, const String& currentStr, const String& prevStr);
};

#endif