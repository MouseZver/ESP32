#ifndef ASYNC_TEMP_SENSOR_H
#define ASYNC_TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>

// Тип для хранения 64-битного адреса датчика
typedef uint8_t DeviceAddress[8];

// Тип функции обратного вызова
typedef void (*TempCallback)(int index, float temp);

class AsyncTempSensor {
public:
    static const int MAX_DEVICES = 8;

    // Конструктор: указываем пин и callback
    AsyncTempSensor(uint8_t pin, TempCallback callback = nullptr);

    // Инициализация: поиск датчиков
    void begin();

    // Обновление температуры (неблокирующий вызов)
    void update(unsigned long interval);

    // Включить/выключить вывод отладочной информации
    void setDebugOutput(bool enabled);

private:
    OneWire _oneWire;
    uint8_t _pin;
    TempCallback _callback = nullptr;

    DeviceAddress _addresses[MAX_DEVICES]; // Адреса найденных датчиков
    int _deviceCount = 0;                  // Количество найденных датчиков

    unsigned long _lastConversionTime = 0; // Время последнего запуска измерения
    bool _conversionStarted = false;       // Флаг: идёт ли измерение
    bool _debug = false;                   // Флаг: включён ли дебаг

    void findDevices();   // Поиск датчиков на шине
    void startConversion(); // Запуск измерения температуры
};

#endif