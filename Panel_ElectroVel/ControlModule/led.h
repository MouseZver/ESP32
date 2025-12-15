/**
 * @file led.h
 * @brief Умная библиотека для управления светодиодами с поддержкой ШИМ и автоматическими паузами
 * @author MouseZver
 * @date 2025
 * 
 * Поддерживаемые платформы:
 * - Arduino Nano/Uno (ATmega328P)
 * - ESP32
 * - Другие Arduino-совместимые платформы
 * 
 * Особенности:
 * - Автоматическое определение поддержки ШИМ
 * - Симуляция ШИМ через digitalWrite для не-ШИМ пинов
 * - Автоматические паузы после завершения анимаций
 * - Современный C++ с оптимизациями
 */

#pragma once

#include <Arduino.h>

/**
 * @class LED
 * @brief Класс для управления светодиодом с расширенными возможностями
 * 
 * Предоставляет методы для:
 * - Плавного затухания (fade)
 * - Двойного мигания (double blink)
 * - Обычного мигания (blink)
 * - Включения/выключения
 * 
 * Автоматически определяет поддержку ШИМ и использует соответствующий метод управления.
 */
class LED final
{
public:
    /**
     * @brief Конструктор класса LED
     * @param pin Номер пина, к которому подключен светодиод
     * @param period Период мигания в миллисекундах (по умолчанию 1000 мс)
     * 
     * @note Автоматически настраивает пин как OUTPUT
     * @note Определяет поддержку ШИМ для выбранного пина
     */
    LED(uint8_t pin, uint16_t period = 1000) noexcept;

    /**
     * @brief Установка периода мигания
     * @param period Новый период в миллисекундах
     * 
     * @note Используется для blink(), а также как пауза после flash() и doubleBlink()
     */
    void setPeriod(uint16_t period) noexcept;

    /**
     * @brief Плавное затухание/нарастание светодиода
     * @param fade Скорость изменения яркости (в миллисекундах на шаг)
     * @param down true для затухания (255→0), false для нарастания (0→255)
     * @return true когда анимация завершена и выполнена пауза, false в процессе
     * 
     * @note Автоматически добавляет паузу длительностью _period после завершения
     * @note Использует ШИМ или симуляцию через digitalWrite
     * 
     * Пример использования:
     * @code
     * if (led.flash(10)) {
     *     // Затухание завершено, пауза выполнена
     * }
     * @endcode
     */
    [[nodiscard]] bool flash(int8_t fade, bool down = true) noexcept;

    /**
     * @brief Двойное мигание светодиодом
     * @param delay Время между включением и выключением в одном цикле
     * @param repeat Количество повторений (0 = бесконечно, 1 = один цикл и т.д.)
     * @return true когда все мигания выполнены и пауза завершена, false в процессе
     * 
     * @note Автоматически добавляет паузу длительностью _period после завершения
     * @note Каждый цикл: включение → delay → выключение → delay
     * 
     * Пример использования:
     * @code
     * if (led.doubleBlink(100, 3)) {
     *     // 3 двойных мигания выполнены, пауза завершена
     * }
     * @endcode
     */
    [[nodiscard]] bool doubleBlink(uint16_t delay, uint16_t repeat = 0) noexcept;

    /**
     * @brief Обычное мигание светодиодом
     * 
     * @note Использует период, установленный через setPeriod()
     * @note Не добавляет автоматическую паузу после выполнения
     */
    void blink() noexcept;

    /**
     * @brief Включение светодиода
     * @param force true для немедленного включения, false для игнорирования
     * 
     * @note Сбрасывает состояние пауз после анимаций
     */
    void on(bool force = true) noexcept;

    /**
     * @brief Выключение светодиодом
     * @param force true для немедленного выключения, false для игнорирования
     * 
     * @note Сбрасывает состояние пауз после анимаций
     */
    void off(bool force = true) noexcept;

    /**
     * @brief Проверка истечения таймера
     * @param delay Пользовательская задержка (0 = использовать _period)
     * @return true если время истекло, false если нет
     * 
     * @note Полезно для создания собственных таймеров
     */
    [[nodiscard]] bool isExpired(uint16_t delay = 0) const noexcept;

private:
    const uint8_t _pin;              ///< Номер пина светодиода
    uint16_t _period;                ///< Период мигания и пауз в мс
    mutable uint32_t _timer;         ///< Основной таймер для анимаций
    uint16_t _repeat;                ///< Счетчик повторений для doubleBlink
    uint8_t _flash;                  ///< Текущее значение яркости для flash
    bool _flag;                      ///< Флаг состояния для blink и doubleBlink
    const bool _pwmCapable;          ///< Поддержка ШИМ для текущего пина
    uint32_t _postDelayTimer;        ///< Таймер для паузы после анимаций
    bool _inPostDelay;               ///< Флаг состояния паузы после анимаций

    /**
     * @brief Определение поддержки ШИМ для пина (compile-time)
     * @param pin Номер пина для проверки
     * @return true если пин поддерживает ШИМ, false если нет
     * 
     * @note Для ESP32 возвращает true для большинства пинов
     * @note Для Arduino Nano/Uno проверяет пины 3,5,6,9,10,11
     */
    static constexpr bool isPWMPin(uint8_t pin) noexcept;

    /**
     * @brief Универсальная запись значения на пин
     * @param value Значение яркости (0-255)
     * 
     * @note Использует analogWrite если пин поддерживает ШИМ
     * @note Использует digitalWrite для симуляции на не-ШИМ пинах
     */
    void writePin(uint8_t value) const noexcept;
};

///////////////////////////////////////////////////////////////////////////////
// Inline реализации
///////////////////////////////////////////////////////////////////////////////

inline LED::LED(uint8_t pin, uint16_t period) noexcept
    : _pin(pin), _period(period), _timer(0), _repeat(0), _flash(0), _flag(false), 
      _pwmCapable(isPWMPin(pin)), _postDelayTimer(0), _inPostDelay(false)
{
    pinMode(_pin, OUTPUT);
}

inline void LED::setPeriod(uint16_t period) noexcept
{
    if (period != _period)
        _period = period;
}

inline bool LED::flash(int8_t fade, bool down) noexcept
{
    const uint32_t now = millis();

    // Если в паузе после завершения
    if (_inPostDelay)
    {
        if (now - _postDelayTimer >= _period)
        {
            _inPostDelay = false;
        }
        return false;
    }

    // Основная логика flash
    const uint32_t elapsed = now - _timer;
    const uint16_t effectiveFade = ((down && _flash > 0) || (!down && _flash < 255)) ? fade : 0;

    if (elapsed >= effectiveFade)
    {
        if (down && _flash == 0)
            _flash = 255;
        else if (!down && _flash == 255)
            _flash = 0;

        _flash += (down ? -1 : 1);

        writePin(_flash);
        _timer = now;

        if (_flash == 0 || _flash == 255)
        {
            // Начинаем паузу после завершения
            _inPostDelay = true;
            _postDelayTimer = now;
            return true;
        }
    }

    return false;
}

inline bool LED::doubleBlink(uint16_t delay, uint16_t repeat) noexcept
{
    const uint32_t now = millis();

    // Если в паузе после завершения
    if (_inPostDelay)
    {
        if (now - _postDelayTimer >= _period)
        {
            _inPostDelay = false;
        }
        return false;
    }

    // Основная логика doubleBlink
    if (now - _timer >= delay)
    {
        _flag = !_flag;
        writePin(_flag ? 255 : 0);
        _timer = now;

        if (!_flag && repeat && ++_repeat >= repeat)
        {
            _repeat = 0;
            // Начинаем паузу после завершения
            _inPostDelay = true;
            _postDelayTimer = now;
            return true;
        }
    }

    return false;
}

inline void LED::blink() noexcept
{
    const uint32_t now = millis();
    if (now - _timer >= _period)
    {
        _flag = !_flag;
        writePin(_flag ? 255 : 0);
        _timer = now;
    }
}

inline void LED::on(bool force) noexcept
{
    if (force)
    {
        _flag = true;
        writePin(255);
        _timer = millis();
        _inPostDelay = false;
    }
}

inline void LED::off(bool force) noexcept
{
    if (force)
    {
        _flag = false;
        writePin(0);
        _timer = millis();
        _inPostDelay = false;
    }
}

inline bool LED::isExpired(uint16_t delay) const noexcept
{
    return (millis() - _timer) >= (delay ? delay : _period);
}

inline constexpr bool LED::isPWMPin(uint8_t pin) noexcept
{
#if defined(ESP32)
    return true;
#elif defined(__AVR_ATmega328P__)
    return (pin == 3 || pin == 5 || pin == 6 || pin == 9 || pin == 10 || pin == 11);
#else
    return true;
#endif
}

inline void LED::writePin(uint8_t value) const noexcept
{
    if (_pwmCapable)
    {
        analogWrite(_pin, value);
    }
    else
    {
        digitalWrite(_pin, value > 0 ? HIGH : LOW);
    }
}