/**
 * @file ButtonManager.cpp
 * @brief Реализация класса ButtonManager для ControlPanel.
 */
#include "ButtonManager.h"

ButtonManager::ButtonManager(int pin) : _button(pin) {}

void ButtonManager::begin(DownClickCheckCallback downCheckCb, SleepToggleCallback sleepToggleCb) {
    _downCheckCb = downCheckCb;
    _sleepToggleCb = sleepToggleCb;

    // 1. Конфигурация кнопок (значения и допуски из оригинальной логики)
    _button.addButton(Button::Up, 2985, 100);
    _button.addButton(Button::Down, 3805, 100);
    _button.addButton(Button::Enter, 4095, 100);

    // 2. Пустые обработчики (сохранено из оригинальной логики)
    _button.onPress([](Button btn) {});
    _button.onRelease([](Button btn) {});

    // 3. Обработка одиночного клика (Click)
    _button.onClick([this](Button btn) {
        switch (btn) {
            case Button::Up:
            case Button::Enter: {
                sendEvent(to_int(btn), 1, to_int(ButtonEvent::Click));
                break;
            }
            case Button::Down: {
                // Оригинальная проверка: отправляем только если мотор не мигает
                if (_downCheckCb && _downCheckCb()) {
                    sendEvent(to_int(btn), 5000, to_int(ButtonEvent::Click));
                }
                break;
            }
        }
    });

    // 4. Обработка удержания (Hold)
    _button.onHold([this](Button btn, uint32_t duration) {
        switch (btn) {
            case Button::Up:
            case Button::Down: {
                sendEvent(to_int(btn), 1, to_int(ButtonEvent::Hold));
                break;
            }
            case Button::Enter: {
                _isSleeping = !_isSleeping; // Переключаем внутреннее состояние
                if (_sleepToggleCb) {
                    _sleepToggleCb(_isSleeping); // Уведомляем основной скетч
                }
                break;
            }
        }
    });

    // 5. Обработка множественного клика (MultiClick)
    _button.onMultiClick([](Button btn, uint8_t count) {
        if (btn == Button::Up && count == 2) {
            sendEvent(to_int(btn), count, to_int(ButtonEvent::MultiClick));
        }
    });
}

void ButtonManager::update() {
    _button.update();
}