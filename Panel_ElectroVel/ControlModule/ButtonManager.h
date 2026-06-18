/**
 * @file ButtonManager.h
 * @brief Класс для обработки входящих команд кнопок и управления соответствующими действиями.
 * 
 * Инкапсулирует логику реакции на события кнопок (включающие изменение состояния пинов, 
 * обновление настроек и отправку событий), получаемых по последовательному порту.
 */
#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include "rom.h" // Для структуры Settings

// === Общие типы для кнопок и событий (перенесены из .ino для переиспользования) ===
enum class Button : int { Up, Down, Enter };
enum class ButtonEvent : int { Press, Click, MultiClick, Hold, Release };
enum class DeviceProperty : int { FrontLight, BackLight, Power, BackLightMode, DateTime, Temperature };

template <typename Enum>
constexpr int to_int(Enum e) { 
    return static_cast<int>(e); 
}

class ButtonManager {
public:
    // Тип callback-функции для отправки событий (используем int для value, так как bool неявно приводится к int, 
    // а в EventUtils.cpp есть инстанцирование sendEvent<int, int>)
    using SendEventCallback = std::function<void(int, int)>;

    /**
     * @brief Конструктор менеджера кнопок.
     * @param settings Ссылка на глобальную структуру настроек для чтения/записи.
     * @param flagPowerBackLight Ссылка на глобальный флаг питания задней подсветки.
     */
    ButtonManager(Settings& settings, bool& flagPowerBackLight);

    /**
     * @brief Установка callback-функции для отправки событий.
     * @param cb Функция, которая будет вызываться для уведомления других систем об изменениях.
     */
    void setSendEventCallback(SendEventCallback cb);

    /**
     * @brief Обработка входящей JSON-команды, представляющей событие кнопки.
     * @param doc Распарсенный JSON-документ с полями "name", "event" и "value".
     */
    void processCommand(JsonDocument& doc);

private:
    Settings& _settings;
    bool& _flagPowerBackLight;
    SendEventCallback _sendEventCb;

    // Локальные определения пинов для инкапсуляции аппаратных деталей
    static const int PIN_IN1 = 32;
    static const int PIN_IN2 = 33;
    static const int PIN_IN3 = 25;
    static const int PIN_IN4 = 26;
    static const int PIN_NEON1 = 13;
    static const int PIN_NEON2 = 14;
    static const int LEDBL_MODE_MAX = 2;
};

#endif // BUTTON_MANAGER_H