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
* @version 1.3
* @date 2025
* @license MIT
*/
#ifndef ANALOGBUTTON_H
#define ANALOGBUTTON_H

#include <Arduino.h>
#include <type_traits>
#include <functional> // <-- ДОБАВЛЕНО для поддержки std::function

struct AnalogButtonConfig {
    uint32_t doubleClickDelay = 150;
    uint32_t holdDelay = 1000;
    uint32_t debounceDelay = 50;
    uint32_t clickTimeout = 200;
};

template<typename ButtonEnum>
class AnalogButton {
    static_assert(std::is_enum_v<ButtonEnum>, "ButtonEnum must be an enum class");
public:
    // <-- ИЗМЕНЕНО: использование std::function вместо указателей на функции
    using PressCallback = std::function<void(ButtonEnum)>;
    using ReleaseCallback = std::function<void(ButtonEnum)>;
    using ClickCallback = std::function<void(ButtonEnum)>;
    using MultiClickCallback = std::function<void(ButtonEnum, uint8_t)>;
    using HoldCallback = std::function<void(ButtonEnum, uint32_t)>;

    AnalogButton(int pin) : _pin(pin) {
        pinMode(_pin, INPUT);
    }

    AnalogButton(int pin, const AnalogButtonConfig& config)
        : _pin(pin), _config(config) {
        pinMode(_pin, INPUT);
    }

    void addButton(ButtonEnum id, uint16_t value, uint16_t tolerance = 100) {
        Button* newButtons = (Button*)realloc(buttons, sizeof(Button) * (buttonCount + 1));
        if (!newButtons) return;
        buttons = newButtons;
        buttons[buttonCount++] = {id, value, tolerance};
    }

    void onPress(PressCallback cb) { _pressCb = cb; }
    void onRelease(ReleaseCallback cb) { _releaseCb = cb; }
    void onClick(ClickCallback cb) { _clickCb = cb; }
    void onMultiClick(MultiClickCallback cb) { _multiClickCb = cb; }
    void onHold(HoldCallback cb) { _holdCb = cb; }

    void update();

private:
    int _pin;
    AnalogButtonConfig _config;

    struct Button {
        ButtonEnum id;
        uint16_t value;
        uint16_t tolerance;
    };

    Button* buttons = nullptr;
    uint8_t buttonCount = 0;

    // Тайминги
    uint32_t lastDebounce = 0;
    uint32_t lastPressTime = 0;
    uint32_t pressStartTime = 0;
    uint8_t clickCount = 0;

    // Состояния
    bool isPressed = false;
    bool wasPressed = false;
    bool isHolding = false;
    ButtonEnum currentButtonId{};

    // Колбэки
    PressCallback _pressCb = nullptr;
    ReleaseCallback _releaseCb = nullptr;
    ClickCallback _clickCb = nullptr;
    MultiClickCallback _multiClickCb = nullptr;
    HoldCallback _holdCb = nullptr;

    int getSmoothedValue() {
        constexpr int ADC_SAMPLES = 7;
        int adcValues[ADC_SAMPLES];
        for (int i = 0; i < ADC_SAMPLES; i++) {
            adcValues[i] = analogRead(_pin);
            delay(1);
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
        return adcValues[ADC_SAMPLES / 2];
    }

    // Возвращает true, если кнопка найдена, и записывает её в `result`
    bool getPressedButton(ButtonEnum& result) {
        int raw = getSmoothedValue();
        if (raw < 100) return false; // ничего не нажато
        for (uint8_t i = 0; i < buttonCount; i++) {
            if (abs(raw - buttons[i].value) <= buttons[i].tolerance) {
                result = buttons[i].id;
                return true;
            }
        }
        return false; // не нашли
    }
};

template<typename ButtonEnum>
void AnalogButton<ButtonEnum>::update() {
    uint32_t now = millis();
    ButtonEnum current{};
    bool hasValidPress = getPressedButton(current);

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