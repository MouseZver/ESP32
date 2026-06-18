/**
 * @file SerialManager.h
 * @brief Класс для управления приемом и обработкой данных по последовательным портам.
 * 
 * Инкапсулирует логику буферизации, таймаутов и парсинга JSON.
 */
#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

using JsonCommandCallback = std::function<void(JsonDocument&)>;

class SerialManager {
public:
    /**
     * @brief Конструктор менеджера последовательного порта.
     * @param commStream Поток для обмена командами (в ControlPanel это Serial2).
     * @param debugStream Поток для отладочного вывода (Serial).
     * @param jsonCallback Функция, вызываемая при успешном парсинге JSON.
     */
    SerialManager(Stream& commStream, Stream& debugStream, JsonCommandCallback jsonCallback);

    /**
     * @brief Основной метод обновления. Должен вызываться в loop().
     */
    void update();

private:
    Stream& _commStream;
    Stream& _debugStream;
    JsonCommandCallback _jsonCallback;

    static const int MAX_MSG_LEN = 128;
    char _incomingBuffer[MAX_MSG_LEN];
    volatile int _bufferIndex = 0;               // Сохранено из оригинальной логики
    volatile unsigned long _lastByteTime = 0;    // Сохранено из оригинальной логики

    void processJsonCommand(const char* jsonStr);
};

#endif // SERIAL_MANAGER_H