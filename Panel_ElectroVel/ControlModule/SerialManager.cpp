/**
 * @file SerialManager.cpp
 * @brief Реализация класса SerialManager для ControlModule.
 */
#include "SerialManager.h"
// #include <StringUtils.h> // Раскомментировать, если библиотека явно подключена в проекте

SerialManager::SerialManager(Stream& commStream, Stream& debugStream, JsonCommandCallback jsonCallback, RawCommandCallback rawCallback)
    : _commStream(commStream), _debugStream(debugStream), _jsonCallback(jsonCallback), _rawCallback(rawCallback) {
    _incomingBuffer[0] = '\0';
}

void SerialManager::update() {
    // 1. Обработка командного потока (JSON через Serial2)
    while (_commStream.available()) {
        char c = _commStream.read();
        if (_bufferIndex < MAX_MSG_LEN - 1) {
            _incomingBuffer[_bufferIndex++] = c;
            _incomingBuffer[_bufferIndex] = '\0';

            if (c == '\n') {
                processJsonCommand(_incomingBuffer);
                _bufferIndex = 0;
                _incomingBuffer[0] = '\0';
            }
            _lastByteTime = millis();
        } else {
            // Буфер переполнен
            _debugStream.println("Buffer overflow!");
            _bufferIndex = 0;
            _incomingBuffer[0] = '\0';
            _lastByteTime = millis();
        }
    }

    // Таймаут незавершённой команды (1000 мс)
    if (_bufferIndex > 0 && (millis() - _lastByteTime > 1000)) {
        _incomingBuffer[_bufferIndex] = '\0';
        processJsonCommand(_incomingBuffer);
        _bufferIndex = 0;
        _incomingBuffer[0] = '\0';
    }

    // 2. Обработка отладочного потока (сырые команды через Serial, например "settime")
    while (_debugStream.available()) {
        String cmd = _debugStream.readStringUntil('\n');
        cmd.trim();
        if (!cmd.isEmpty() && _rawCallback) {
            _rawCallback(cmd);
        }
    }
}

void SerialManager::processJsonCommand(const char* jsonStr) {
    if (!_jsonCallback) return;

    String str = jsonStr;
    // str = StringUtils::trim(str); // Оригинальная логика, если библиотека подключена
    str.trim(); // Стандартный метод Arduino, полностью заменяет StringUtils::trim для этой задачи

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, str);

    _debugStream.println("GET:");
    _debugStream.println(str);

    if (error) {
        _debugStream.print("JSON parsing error: ");
        _debugStream.println(error.f_str());
        return;
    }

    _jsonCallback(doc);
}