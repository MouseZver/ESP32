/**
 * @file ButtonManager.h
 * @brief Класс для управления аналоговыми кнопками и обработки событий нажатий.
 * 
 * Инкапсулирует логику конфигурации AnalogButton, тайминги и реакцию на события,
 * передавая специфичную бизнес-логику (например, управление сном) через callback-функции.
 */
#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include <functional>

// === Общие типы для кнопок и событий (перенесены из .ino для переиспользования) ===
enum class Button : int { Up, Down, Enter };
enum class ButtonEvent : int { Press, Click, MultiClick, Hold, Release };
enum class DeviceProperty : int { FrontLight, BackLight, Power, BackLightMode, DateTime, Temperature };

template <typename Enum>
constexpr int to_int(Enum e) { 
    return static_cast<int>(e); 
}

#include "AnalogButton.h"
#include "EventUtils.h" // Для функции sendEvent

class ButtonManager {
public:
    // Типы callback-функций для связи с основной логикой скетча
    using DownClickCheckCallback = std::function<bool()>;       // Проверка перед отправкой клика Down
    using SleepToggleCallback = std::function<void(bool)>;      // Уведомление о переключении режима сна

    /**
     * @brief Конструктор менеджера кнопок.
     * @param pin Аналоговый пин, к которому подключен резистивный делитель кнопок.
     */
    ButtonManager(int pin);

    /**
     * @brief Инициализация и настройка обработчиков событий.
     * @param downCheckCb Функция, возвращающая true, если клик Down разрешен.
     * @param sleepToggleCb Функция, вызываемая при переключении режима сна (передает новое состояние).
     */
    void begin(DownClickCheckCallback downCheckCb, SleepToggleCallback sleepToggleCb);

    /**
     * @brief Основной метод обновления. Должен вызываться в loop().
     */
    void update();

    /**
     * @brief Проверка текущего состояния режима сна.
     * @return true, если устройство в режиме сна, иначе false.
     */
    bool isSleeping() const { return _isSleeping; }

private:
    AnalogButton<Button> _button;
    DownClickCheckCallback _downCheckCb;
    SleepToggleCallback _sleepToggleCb;
    bool _isSleeping = false;
};

#endif // BUTTON_MANAGER_H