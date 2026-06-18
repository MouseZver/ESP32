/**
 * @file ButtonManager.cpp
 * @brief Реализация класса ButtonManager для ControlModule.
 * 
 * Содержит точную копию оригинальной логики обработки событий кнопок,
 * но с использованием инкапсулированных ссылок на состояние и callback-функций.
 */
#include "ButtonManager.h"

ButtonManager::ButtonManager(Settings& settings, bool& flagPowerBackLight)
    : _settings(settings), _flagPowerBackLight(flagPowerBackLight), _sendEventCb(nullptr) {}

void ButtonManager::setSendEventCallback(SendEventCallback cb) {
    _sendEventCb = cb;
}

void ButtonManager::processCommand(JsonDocument& doc) {
    if (!doc["name"].is<int>() || !doc["event"].is<int>()) {
        Serial.println("Error: 'name' or 'event' is not an integer");
        return;
    }
    int name = doc["name"].as<int>();
    int event = doc["event"].as<int>();
    int value = doc["value"].as<int>();

    if (name < to_int(Button::Up) || name > to_int(Button::Enter)) {
        Serial.printf("Invalid button ID: %d\n", name);
        return;
    }
    Button button = static_cast<Button>(name);
    ButtonEvent buttonEvent = static_cast<ButtonEvent>(event);

    switch (button) {
        case Button::Up: {
            switch (buttonEvent) {
                case ButtonEvent::Click: {
                    bool currentState = digitalRead(PIN_IN4);
                    bool newState = !currentState;
                    digitalWrite(PIN_IN4, newState);
                    _settings.frontLight = newState;
                    if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::FrontLight), newState ? 1 : 0);
                    break;
                }
                case ButtonEvent::MultiClick: {
                    if (value == 2) {
                        _settings.backLightMode++;
                        if (_settings.backLightMode > LEDBL_MODE_MAX) {
                            _settings.backLightMode = 1;
                        }
                        if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::BackLightMode), _settings.backLightMode);
                    }
                    break;
                }
                case ButtonEvent::Hold: {
                    _flagPowerBackLight = !_flagPowerBackLight;
                    if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::BackLight), _flagPowerBackLight ? 1 : 0);
                    break;
                }
                default: break;
            }
            break;
        }
        case Button::Down: {
            switch (buttonEvent) {
                case ButtonEvent::Click: {
                    int p = digitalRead(PIN_IN2);
                    if (p) {
                        digitalWrite(PIN_IN2, !p);
                        if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 1);
                        delay(value);
                    } else {
                        if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 0);
                    }
                    digitalWrite(PIN_IN3, !p);
                    if (p) {
                        if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 1);
                    } else {
                        delay(value);
                        digitalWrite(PIN_IN2, HIGH);
                        if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 0);
                    }
                    break;
                }
                case ButtonEvent::Hold: {
                    int n = digitalRead(PIN_NEON1);
                    digitalWrite(PIN_NEON1, !n);
                    digitalWrite(PIN_NEON2, !n);
                    break;
                }
                default: break;
            }
            break;
        }
        case Button::Enter: {
            if (buttonEvent == ButtonEvent::Click) {
                bool p = digitalRead(PIN_IN2);
                if (!p) {
                    digitalWrite(PIN_IN3, HIGH);
                    if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 0);
                    delay(1000);
                    digitalWrite(PIN_IN2, HIGH);
                    if (_sendEventCb) _sendEventCb(to_int(DeviceProperty::Power), 0);
                }
                saveSettings(_settings); // Функция из rom.h
                digitalWrite(PIN_IN1, HIGH);
            }
            break;
        }
        default:
            Serial.println("Unknown button (should not happen)");
            break;
    }
}