enum class Button : int {
    Up,     // 0
    Down,   // 1
    Enter   // 2
};

enum class ButtonEvent : int {
    Press,
    Click,
    MultiClick,
    Hold,
    Release
};

enum class DeviceProperty : int {
    FrontLight,
    BackLight,
    Power,
    BackLightMode,
    DateTime,
    Temperature,
};

// to_int
template <typename Enum>
constexpr int to_int(Enum e) {
    return static_cast<int>(e);
}

#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "EventUtils.h"
#include "DisplayManager.h"
#include "AnalogButton.h"
#include "BezierAnimation.h"

#define RX_PIN 18
#define TX_PIN 17

// Шаблонный AnalogButton<Button>
AnalogButton<Button> button(1);

TFT_eSPI tft = TFT_eSPI();
DisplayManager display(tft);
BezierAnimation bezierAnim;
bool _sleep = false;

#include <terminal.h>

void setup() {
    Serial.begin(115200);
    Serial2.begin(4800, SERIAL_8N1, RX_PIN, TX_PIN);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    display.begin();
    bezierAnim.begin();
    display.setTemperature(00.0);

    // Добавляем кнопки с enum
    button.addButton(Button::Up,     2985, 100); // Up
    button.addButton(Button::Down,   3805, 100); // Down
    button.addButton(Button::Enter,  4095, 100); // Enter

    // Колбэки с enum
    button.onPress([](Button btn) {
        // можно логировать
        //Serial.println(1);
    });

    button.onClick([](Button btn) {
        switch (btn) {
            case Button::Up:
            case Button::Enter: {
                sendEvent(to_int(btn), 1, to_int(ButtonEvent::Click));
                break;
            }
            case Button::Down: {
                if (!display.getMotorBlinkStatus()) {
                    sendEvent(to_int(btn), 5000, to_int(ButtonEvent::Click));
                }
                break;
            }
        }
    });

    button.onHold([](Button btn, uint32_t duration) {
        switch (btn) {
            case Button::Up:
            case Button::Down: {
                sendEvent(to_int(btn), 1, to_int(ButtonEvent::Hold));
                break;
            }
            case Button::Enter: {
                _sleep = !_sleep;
                if (!_sleep) display.begin();
                tft.fillScreen(TFT_BLACK);
                break;
            }
        }
    });

    button.onMultiClick([](Button btn, uint8_t count) {
        if (btn == Button::Up && count == 2) {
            sendEvent(to_int(btn), count, to_int(ButtonEvent::MultiClick));
        }
    });

    button.onRelease([](Button btn) {
        // можно игнорировать
    });
}

void loop() {
    serialEvent();
    button.update();

    if (_sleep) {
        bezierAnim.update();
        bezierAnim.draw();
    } else {
        display.update();
    }
}