// ============================================================================
// 1. Директивы #include (включая новый ButtonManager.h)
// ============================================================================
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "EventUtils.h"
#include "DisplayManager.h"
#include "BezierAnimation.h"
#include "SerialManager.h"
#include "ButtonManager.h" // <-- Подключаем новый менеджер кнопок

#define RX_PIN 18
#define TX_PIN 17

// ============================================================================
// 2. Глобальные объекты и экземпляры менеджеров
// ============================================================================
TFT_eSPI tft = TFT_eSPI();
DisplayManager display(tft);
BezierAnimation bezierAnim;

// Предварительное объявление функции обработчика команд (чтобы она была видна при инициализации serialManager)
void handleJsonCommand(JsonDocument& doc);

// Менеджеры (инкапсулируют всю внутреннюю логику)
SerialManager serialManager(Serial2, Serial, handleJsonCommand);
ButtonManager buttonManager(1); // Пин 1 для аналоговых кнопок

// ============================================================================
// 3. Обработчики команд
// ============================================================================
void handleJsonCommand(JsonDocument& doc) {
    bool value = doc["value"].as<bool>();
    Serial.println("commandsTerminal:");
    Serial.println(String(doc["name"]));
    Serial.println(String(value));
    
    switch (doc["name"].as<int>()) {
        case 0:
            display.setLampStatus(value);
            break;
        case 1:
            display.setBackLampStatus(value);
            break;
        case 2:
            display.setMotorStatus(value);
            break;
        case 3:
            // modeBackLamp
            break;
        case 4:
            display.setTime(doc["value"].as<uint32_t>());
            break;
        case 5:
            display.setTemperature(doc["value"].as<float>());
            break;
        default:
            break;
    }
}

// ============================================================================
// 4. Основные функции setup и loop
// ============================================================================
void setup() {
    Serial.begin(115200);
    Serial2.begin(4800, SERIAL_8N1, RX_PIN, TX_PIN);
    
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
    display.begin();
    bezierAnim.begin();
    display.setTemperature(00.0);
    
    // Инициализация менеджера кнопок с передачей специфичной логики через лямбда-функции
    buttonManager.begin(
        // DownClickCheckCallback: проверяем, можно ли отправить событие Down
        []() {
            return !display.getMotorBlinkStatus();
        },
        // SleepToggleCallback: реакция на переключение режима сна
        [](bool isSleeping) {
            if (!isSleeping) {
                display.begin(); // Пробуждение: инициализируем дисплей заново
            }
            tft.fillScreen(TFT_BLACK); // Очищаем экран при любом изменении состояния сна
        }
    );
}

void loop() {
    serialManager.update();
    buttonManager.update(); // <-- Обновление состояния кнопок
    
    // Логика отрисовки в зависимости от режима сна (управляется через buttonManager.isSleeping())
    if (buttonManager.isSleeping()) {
        bezierAnim.update();
        bezierAnim.draw();
    } else {
        display.update();
    }
}