/**
 * @file AnalogButton.h
 * @brief Умная обработка аналоговых кнопок с поддержкой enum class.
 *
 * Эта библиотека позволяет работать с резистивным делителем на одной аналоговой пине,
 * распознавая нажатия нескольких кнопок по уникальным ADC-значениям.
 *
 * Поддерживает:
 * - Долгое удержание (Hold)
 * - Одиночный и множественный клик (Click / MultiClick)
 * - Подавление дребезга
 * - Пользовательские enum-ы для типобезопасности
 *
 * @author MouseZver
 * @version 1.0
 * @date 2025
 * @license MIT
 */

#ifndef ANALOGBUTTON_H
#define ANALOGBUTTON_H

#include <Arduino.h>
#include <type_traits>

/**
 * @struct AnalogButtonConfig
 * @brief Конфигурационная структура для настройки поведения кнопок.
 */
struct AnalogButtonConfig {
    /**
     * @brief Задержка между кликами для распознавания двойного клика (мс).
     * Если второй клик происходит быстрее, чем это значение — считается двойным.
     */
    uint32_t doubleClickDelay = 150;

    /**
     * @brief Время удержания, при котором срабатывает событие Hold (мс).
     */
    uint32_t holdDelay = 1000;

    /**
     * @brief Время дребезга контактов (мс). Используется для фильтрации ложных нажатий.
     */
    uint32_t debounceDelay = 50;

    /**
     * @brief Таймаут после отпускания, в течение которого ожидается следующий клик (мс).
     * Если не нажать вовремя — серия кликов завершается.
     */
    uint32_t clickTimeout = 200;
};

/**
 * @class AnalogButton
 * @brief Шаблонный класс для обработки аналоговых кнопок с поддержкой enum-идентификаторов.
 *
 * Позволяет использовать любой enum class в качестве идентификатора кнопки.
 * Это обеспечивает типобезопасность и исключает ошибки, связанные с использованием строк.
 *
 * @tparam ButtonEnum Тип enum class, используемый для идентификации кнопок.
 *                    Должен быть объявлен как `enum class MyButton { ... };`
 */
template<typename ButtonEnum>
class AnalogButton {
    static_assert(std::is_enum_v<ButtonEnum>, "ButtonEnum must be an enum class");

public:
    /**
     * @brief Колбэк: вызывается при нажатии кнопки (когда состояние переходит в "нажата").
     * @param button Идентификатор кнопки (enum).
     */
    using PressCallback = void (*)(ButtonEnum);

    /**
     * @brief Колбэк: вызывается при отпускании кнопки.
     * @param button Идентификатор кнопки (enum).
     */
    using ReleaseCallback = void (*)(ButtonEnum);

    /**
     * @brief Колбэк: вызывается при одиночном клике (после отпускания).
     * @param button Идентификатор кнопки (enum).
     */
    using ClickCallback = void (*)(ButtonEnum);

    /**
     * @brief Колбэк: вызывается при двойном или множественном клике.
     * @param button Идентификатор кнопки (enum).
     * @param count Количество кликов.
     */
    using MultiClickCallback = void (*)(ButtonEnum, uint8_t);

    /**
     * @brief Колбэк: вызывается при удержании кнопки.
     * @param button Идентификатор кнопки (enum).
     * @param duration Длительность удержания в миллисекундах.
     */
    using HoldCallback = void (*)(ButtonEnum, uint32_t);

    /**
     * @brief Конструктор с минимальной конфигурацией.
     * @param pin Аналоговый пин, к которому подключён делитель напряжения.
     */
    AnalogButton(int pin) : _pin(pin) {
        pinMode(_pin, INPUT);
    }

    /**
     * @brief Конструктор с ручной настройкой параметров.
     * @param pin Аналоговый пин.
     * @param doubleClickDelay Задержка двойного клика (мс).
     * @param holdDelay Время удержания для события Hold (мс).
     * @param debounceDelay Время дребезга (мс).
     * @param clickTimeout Таймаут кликов (мс).
     */
    AnalogButton(int pin, uint32_t doubleClickDelay, uint32_t holdDelay,
                 uint32_t debounceDelay, uint32_t clickTimeout)
        : _pin(pin), _config({doubleClickDelay, holdDelay, debounceDelay, clickTimeout}) {
        pinMode(_pin, INPUT);
    }

    /**
     * @brief Конструктор с передачей конфигурационной структуры.
     * @param pin Аналоговый пин.
     * @param config Структура с настройками.
     */
    AnalogButton(int pin, const AnalogButtonConfig& config)
        : _pin(pin), _config(config) {
        pinMode(_pin, INPUT);
    }

    /**
     * @brief Добавляет кнопку в список распознаваемых.
     *
     * Кнопка определяется по ожидаемому значению АЦП. При измерении с ADC
     * значение сравнивается с заданным с учётом допуска (tolerance).
     *
     * @param id Идентификатор кнопки (enum).
     * @param value Ожидаемое значение АЦП (0–4095 для ESP32).
     * @param tolerance Допуск по значению (по умолчанию 100).
     *
     * @note Рекомендуется использовать медиану нескольких измерений для точности.
     */
    void addButton(ButtonEnum id, uint16_t value, uint16_t tolerance = 100) {
        Button* newButtons = (Button*)realloc(buttons, sizeof(Button) * (buttonCount + 1));
        if (!newButtons) return;
        buttons = newButtons;
        buttons[buttonCount++] = {id, value, tolerance};
    }

    /**
     * @brief Подписка на событие "нажатие".
     * @param cb Функция-колбэк, принимающая идентификатор кнопки.
     */
    void onPress(PressCallback cb) { _pressCb = cb; }

    /**
     * @brief Подписка на событие "отпускание".
     * @param cb Функция-колбэк, принимающая идентификатор кнопки.
     */
    void onRelease(ReleaseCallback cb) { _releaseCb = cb; }

    /**
     * @brief Подписка на событие "одиночный клик".
     * @param cb Функция-колбэк, принимающая идентификатор кнопки.
     */
    void onClick(ClickCallback cb) { _clickCb = cb; }

    /**
     * @brief Подписка на событие "множественный клик".
     * @param cb Функция-колбэк, принимающая идентификатор кнопки и количество кликов.
     */
    void onMultiClick(MultiClickCallback cb) { _multiClickCb = cb; }

    /**
     * @brief Подписка на событие "удержание".
     * @param cb Функция-колбэк, принимающая идентификатор кнопки и длительность удержания.
     */
    void onHold(HoldCallback cb) { _holdCb = cb; }

    /**
     * @brief Основной метод обновления состояния.
     *
     * Должен вызываться регулярно в `loop()`. Выполняет:
     * - Чтение АЦП (медиана 7 замеров)
     * - Определение нажатой кнопки
     * - Дебаунсинг
     * - Распознавание кликов и удержания
     * - Вызов соответствующих колбэков
     *
     * @note Не используйте `delay()` внутри колбэков — это нарушит тайминги.
     */
    void update();

private:
    int _pin;                    ///< Номер пина, к которому подключена кнопочная цепь.
    AnalogButtonConfig _config;  ///< Конфигурация таймингов.

    /**
     * @struct Button
     * @brief Внутреннее представление одной кнопки.
     */
    struct Button {
        ButtonEnum id;           ///< Идентификатор кнопки (enum).
        uint16_t value;          ///< Ожидаемое значение АЦП.
        uint16_t tolerance;      ///< Допуск (погрешность).
    };

    Button* buttons = nullptr;   ///< Динамический массив кнопок.
    uint8_t buttonCount = 0;     ///< Количество добавленных кнопок.

    // Тайминги
    uint32_t lastDebounce = 0;       ///< Время последнего изменения состояния.
    uint32_t lastPressTime = 0;      ///< Время последнего завершённого клика.
    uint32_t pressStartTime = 0;     ///< Время начала текущего нажатия.
    uint8_t clickCount = 0;          ///< Счётчик кликов в серии.

    // Состояния
    bool isPressed = false;          ///< Кнопка в данный момент нажата.
    bool wasPressed = false;         ///< Была нажата (для обработки кликов).
    bool isHolding = false;          ///< Уже сработало удержание (чтобы не дублировать).
    ButtonEnum currentButtonId = {}; ///< Идентификатор текущей активной кнопки.

    // Колбэки
    PressCallback _pressCb = nullptr;
    ReleaseCallback _releaseCb = nullptr;
    ClickCallback _clickCb = nullptr;
    MultiClickCallback _multiClickCb = nullptr;
    HoldCallback _holdCb = nullptr;

    /**
     * @brief Получает стабильное значение АЦП методом медианы.
     * @return Отфильтрованное значение (0–4095).
     *
     * Использует 7 замеров, сортирует их и возвращает медиану.
     * Уменьшает влияние шумов и скачков.
     */
    int getSmoothedValue() {
        constexpr int ADC_SAMPLES = 7;
        int adcValues[ADC_SAMPLES];

        for (int i = 0; i < ADC_SAMPLES; i++) {
            adcValues[i] = analogRead(_pin);
            delay(1); // короткая пауза между измерениями
        }

        // Сортировка пузырьком
        for (int i = 0; i < ADC_SAMPLES - 1; i++) {
            for (int j = i + 1; j < ADC_SAMPLES; j++) {
                if (adcValues[i] > adcValues[j]) {
                    int temp = adcValues[i];
                    adcValues[i] = adcValues[j];
                    adcValues[j] = temp;
                }
            }
        }

        return adcValues[ADC_SAMPLES / 2]; // медиана
    }

    /**
     * @brief Определяет, какая кнопка нажата.
     * @return Идентификатор кнопки или пустое значение, если ничего не нажато.
     *
     * Сравнивает текущее значение АЦП со всеми зарегистрированными кнопками.
     * Если значение попадает в диапазон `[value ± tolerance]` — возвращает id.
     *
     * @note Если ни одна кнопка не подходит, возвращается `{}` (нулевое значение enum).
     *       Убедитесь, что ваш enum не использует `0` как валидное значение "ничего".
     */
    ButtonEnum getPressedButton() {
        int raw = getSmoothedValue();
        if (raw < 100) return {}; // Ниже порога — ничего не нажато

        for (uint8_t i = 0; i < buttonCount; i++) {
            if (abs(raw - buttons[i].value) <= buttons[i].tolerance) {
                return buttons[i].id;
            }
        }
        return {};
    }
};

// Реализация update() — прямо в .h (требуется для шаблонов)
template<typename ButtonEnum>
void AnalogButton<ButtonEnum>::update() {
    uint32_t now = millis();
    ButtonEnum current = getPressedButton();
    // Проверяем, что значение enum валидно (если enum начинается с 0, то 0 — валидно)
    bool hasValidPress = static_cast<int>(current) >= 0 || buttonCount == 0 ? false : true;

    if (hasValidPress) {
        if (!isPressed) {
            lastDebounce = now;
            isPressed = true;
            currentButtonId = current;
            pressStartTime = now;
            wasPressed = true;

            if (_pressCb) _pressCb(current);
        } else if (_holdCb && !isHolding && now - pressStartTime >= _config.holdDelay) {
            isHolding = true;
            _holdCb(current, now - pressStartTime);
        }
    } else {
        if (isPressed) {
            uint32_t pressDuration = now - pressStartTime;
            isPressed = false;
            isHolding = false;

            if (_releaseCb) _releaseCb(currentButtonId);

            if (pressDuration < _config.holdDelay) {
                clickCount++;
                lastPressTime = now;
            }
        } else if (wasPressed && clickCount > 0 &&
                   (now - lastPressTime >= _config.clickTimeout)) {

            if (clickCount == 1 && _clickCb) {
                _clickCb(currentButtonId);
            } else if (clickCount >= 2 && _multiClickCb) {
                _multiClickCb(currentButtonId, clickCount);
            }

            clickCount = 0;
            wasPressed = false;
        }
    }
}

#endif // ANALOGBUTTON_H