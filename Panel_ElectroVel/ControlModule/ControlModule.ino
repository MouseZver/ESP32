/*
frontLight - 0
backLight - 1
power - 2
modeBackLight - 3
date - 4
temperature - 5
*/

// ============================================================================
// 1. ВСЕ директивы #include ДОЛЖНЫ быть в самом верху файла!
// ============================================================================
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <BluetoothSerial.h>
#include "EventUtils.h"
#include "led.h"
#include "rom.h"
#include "AsyncTempSensor.h"
#include "SerialManager.h"
#include "ButtonManager.h" // <-- Подключаем новый менеджер кнопок

// ============================================================================
// 2. Глобальные переменные и определения пинов
// ============================================================================
#define RX_PIN 16
#define TX_PIN 17
#define BAUD_RATE 115200

int pins[] = { 26, 25, 33, 32 }; // IN4, IN3, IN2, IN1

bool _flashBlink = true;
bool _flagPowerBackLight = false;  

#define PIN_POWER_BACK_LIGHT 15

LED ledPing(2, 500);
LED ledBL(PIN_POWER_BACK_LIGHT, 500);

AsyncTempSensor tempSensor(4, [](int index, float temp) {
    sendEvent(to_int(DeviceProperty::Temperature), temp);
});

RTC_DS3231 rtc;
Settings deviceSettings;

// ============================================================================
// 3. Экземпляры менеджеров
// ============================================================================
// Менеджер кнопок инициализируется ссылками на глобальные переменные, которые он будет изменять
ButtonManager buttonManager(deviceSettings, _flagPowerBackLight);

// ============================================================================
// 4. Callback-функции для SerialManager
// ============================================================================
void onJsonReceived(JsonDocument& doc) {
    // Делегируем обработку JSON-команды менеджеру кнопок
    buttonManager.processCommand(doc);
}

void onRawCommand(const String& cmd) {
    if (cmd.startsWith("settime ")) {
        String timeStr = cmd.substring(8);
        if (timeStr.length() == 19) {
            int Y = timeStr.substring(0, 4).toInt();
            int M = timeStr.substring(5, 7).toInt();
            int D = timeStr.substring(8, 10).toInt();
            int h = timeStr.substring(11, 13).toInt();
            int m = timeStr.substring(14, 16).toInt();
            int s = timeStr.substring(17, 19).toInt();
            DateTime dt(Y, M, D, h, m, s);
            rtc.adjust(dt);
            Serial.println("RTC set to: " + String(dt.timestamp()));
        } else {
            Serial.println("Invalid format. Use: settime YYYY-MM-DD HH:MM:SS");
        }
    }
}

SerialManager serialManager(Serial2, Serial, onJsonReceived, onRawCommand);

// ============================================================================
// 5. Основные функции setup и loop
// ============================================================================
void setup() {
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("EEPROM: не удалось инициализировать!");
    }
    loadSettings(deviceSettings);
    
    for (int pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    
    pinMode(13, OUTPUT); // NEON1
    digitalWrite(13, LOW);
    pinMode(14, OUTPUT); // NEON2
    digitalWrite(14, LOW);
    
    digitalWrite(32, 0); // IN1
    digitalWrite(26, deviceSettings.frontLight); // IN4
    
    Serial.begin(BAUD_RATE);
    Serial2.begin(4800, SERIAL_8N1, RX_PIN, TX_PIN);
    
    tempSensor.begin();
    Wire.begin();
    rtc.begin();
    
    WiFi.mode(WIFI_OFF);
    btStop();
    WiFi.disconnect(true);
    delay(2000);
    
    // Настраиваем callback для отправки событий из ButtonManager
    buttonManager.setSendEventCallback([](int property, int value) {
        sendEvent(property, value);
    });

    sendEvent(to_int(DeviceProperty::FrontLight), !deviceSettings.frontLight);
    sendEvent(to_int(DeviceProperty::BackLight), _flagPowerBackLight);
    
    DateTime now = rtc.now();
    sendEvent(to_int(DeviceProperty::DateTime), now.unixtime());
    
    Serial.println("🙂");
}

void loop() {
    serialManager.update();
    tempSensor.update(3000);
    
    if (_flagPowerBackLight) {
        ledBL.setPeriod(500);
        ledPing.setPeriod(500);
        switch (deviceSettings.backLightMode) {
            default:
            case 0x1: {
                ledBL.setPeriod(1500);
                ledPing.setPeriod(1500);
                (void)ledBL.doubleBlink(100, 1);
                (void)ledPing.doubleBlink(100, 1);
                break;
            }
            case 0x2: {
                (void)ledBL.doubleBlink(80, 8);
                (void)ledPing.doubleBlink(80, 8);
                break;
            }
        }
    } else if (digitalRead(PIN_POWER_BACK_LIGHT)) {
        ledBL.off();
        ledPing.off();
    }
    
    if (!_flagPowerBackLight) (void)ledPing.doubleBlink(50, 10);
}