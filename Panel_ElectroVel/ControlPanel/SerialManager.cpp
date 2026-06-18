/**
 * @file SerialManager.cpp
 * @brief Реализация класса SerialManager для ControlPanel.
 *
 * Инкапсулирует логику буферизации, таймаутов и парсинга JSON.
 * 
 * Исправление: Использование встроенного метода String::trim() вместо 
 * StringUtils::trim(). Это предотвращает ошибку линковки (undefined reference), 
 * возникающую из-за того, что Arduino IDE не компилирует .cpp файлы из 
 * вложенных папок без специальной настройки. Данная логика полностью 
 * идентична рабочей реализации в ControlModule\SerialManager.cpp.
 */
#include "SerialManager.h"
// #include "StringUtils/StringUtils.h" // Закомментировано для предотвращения ошибки линковки

SerialManager::SerialManager(Stream& commStream, Stream& debugStream, JsonCommandCallback jsonCallback)
: _commStream(commStream), _debugStream(debugStream), _jsonCallback(jsonCallback) {
    _incomingBuffer[0] = '\0';
}

void SerialManager::update() {
    // Обработка командного потока (в оригинале это был Serial2 внутри функции serialEvent)
    while (_commStream.available()) {
        char c = _commStream.read();
        if (_bufferIndex < MAX_MSG_LEN - 1) {
            _incomingBuffer[_bufferIndex] = c;
            
            // Оригинальная логика с прерываниями сохранена в соответствии с Правилом 1
            noInterrupts();
            uint8_t current = _bufferIndex;
            current++;
            _bufferIndex = current;
            interrupts();
            
            _incomingBuffer[_bufferIndex] = '\0';
            
            // Проверка на символ новой строки (конец JSON-сообщения)
            if (c == '\n') {
                processJsonCommand(_incomingBuffer);
                _bufferIndex = 0;
                _incomingBuffer[0] = '\0';
            }
            _lastByteTime = millis();
        } else {
            // Буфер переполнен: сбрасываем указатель, чтобы избежать повреждения памяти
            _bufferIndex = 0;
            _incomingBuffer[0] = '\0';
            _lastByteTime = millis();
        }
    }
    
    // Таймаут незавершённой команды (1000 мс): если данные перестали поступать, 
    // считаем команду завершенной и обрабатываем её.
    if (_bufferIndex > 0 && (millis() - _lastByteTime > 1000)) {
        _incomingBuffer[_bufferIndex] = '\0';
        processJsonCommand(_incomingBuffer);
        _bufferIndex = 0;
        _incomingBuffer[0] = '\0';
    }
}

void SerialManager::processJsonCommand(const char* jsonStr) {
    if (!_jsonCallback) return;
    
    String str = jsonStr;
    
    // str = StringUtils::trim(str); // Оригинальная логика, если библиотека подключена
    str.trim(); // Стандартный метод Arduino, полностью заменяет StringUtils::trim для этой задачи
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, str);
    
    _debugStream.println("Пришло:");
    _debugStream.println(str);
    
    if (error) {
        _debugStream.println("Ошибка разбора JSON");
        return;
    }
    
    // Передаем успешно распарсенный документ в callback-функцию основного скетча
    _jsonCallback(doc);
}