/**
 * @file SerialManager.h
 * @brief Класс для управления приемом и обработкой данных по последовательным портам.
 * 
 * Инкапсулирует логику буферизации, таймаутов и парсинга JSON,
 * передавая готовые данные в callback-функции, определенные в основном скетче.
 */
#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

// Тип функции обратного вызова для обработки распарсенных JSON команд
using JsonCommandCallback = std::function<void(JsonDocument&)>;
// Тип функции обратного вызова для обработки сырых текстовых команд (например, "settime")
using RawCommandCallback = std::function<void(const String&)>;

class SerialManager {
public:
    /**
     * @brief Конструктор менеджера последовательного порта.
     * @param commStream Поток для обмена командами (обычно Serial2).
     * @param debugStream Поток для отладочного вывода и сырых команд (обычно Serial).
     * @param jsonCallback Функция, вызываемая при успешном парсинге JSON.
     * @param rawCallback Функция, вызываемая при получении сырой строки из debugStream.
     */
    SerialManager(Stream& commStream, Stream& debugStream, JsonCommandCallback jsonCallback, RawCommandCallback rawCallback);

    /**
     * @brief Основной метод обновления. Должен вызываться в loop().
     * Проверяет наличие данных в обоих потоках и обрабатывает их.
     */
    void update();

private:
    Stream& _commStream;
    Stream& _debugStream;
    JsonCommandCallback _jsonCallback;
    RawCommandCallback _rawCallback;

    static const int MAX_MSG_LEN = 128;
    char _incomingBuffer[MAX_MSG_LEN];
    int _bufferIndex = 0;
    unsigned long _lastByteTime = 0;

    void processJsonCommand(const char* jsonStr);
};

#endif // SERIAL_MANAGER_H