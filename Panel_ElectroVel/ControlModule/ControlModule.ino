/*
frontLight - 0
backLight - 1
power - 2
modeBackLight - 3
date - 4
temperature - 5
*/

enum class Button : int {
    Up,
    Down,
    Enter
};

enum class ButtonEvent : int {
    Press,        // кнопка нажата (в момент нажатия)
    Click,        // одиночный клик (после отпускания)
    MultiClick,   // двойной/множественный клик
    Hold,         // удержание
    Release       // отпускание
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

//#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <BluetoothSerial.h>
#include "EventUtils.h"
#include "led.h"
#include "rom.h"
#include "AsyncTempSensor.h"

// Пины UART
#define RX_PIN 16
#define TX_PIN 17
#define BAUD_RATE 115200
// Пины для взаимодействия с сигналами реле
#define IN1 32  // общее включение питание
#define IN2 33  // питание через резистор для подзарядки конденсаторов контроллера
#define IN3 25  // питание "силовое реле" для контроллера
#define IN4 26  // питание переднего фонаря
// Неоновые колеса
#define NEON1 13
#define NEON2 14

int pins[] = { IN4, IN3, IN2, IN1 };
bool _flashBlink = true;
bool _flagPowerBackLight = false;  //bool PIN_INDICATOR_BACK_LIGHT_LED

// пин PWD 3. Питание через мосфет для заднего фонаря
#define PIN_POWER_BACK_LIGHT 15

// максимальное кол-во выбора режимов для сигнала заднего фонаря
#define LEDBL_MODE_MAX 2

// связь с esp Serial
// SoftwareSerial Serial2(RX_PIN, TX_PIN);

// PIN 2 процесс работы
LED ledPing(2, 500);

// PIN 15 через мосфет. Задний фонарь
LED ledBL(PIN_POWER_BACK_LIGHT, 500);

// PIN 4 для DATA линии DS18B20
AsyncTempSensor tempSensor(4, [](int index, float temp) {
	sendEvent(to_int(DeviceProperty::Temperature), temp);
});

RTC_DS3231 rtc;

Settings deviceSettings;

#include "terminal.h"

void setup() {
    // Инициализация EEPROM для ESP32
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("EEPROM: не удалось инициализировать!");
    }
	
	loadSettings(deviceSettings);

	for (int pin : pins) {
		pinMode(pin, OUTPUT);

		digitalWrite(pin, HIGH);
	}
	
	// Неоновые колеса
	pinMode(NEON1, OUTPUT);
	digitalWrite(NEON1, LOW);
	pinMode(NEON2, OUTPUT);
	digitalWrite(NEON2, LOW);

	digitalWrite(IN1, 0);
	digitalWrite(IN4, deviceSettings.frontLight);

	Serial.begin(BAUD_RATE);
	Serial2.begin(4800, SERIAL_8N1, RX_PIN, TX_PIN);
	tempSensor.begin();
	Wire.begin();
	rtc.begin();

	// Отключаем WiFi и Bluetooth
	WiFi.mode(WIFI_OFF);
	btStop();

	// Дополнительно - можно ещё отключить WiFi полностью
	WiFi.disconnect(true);

	delay(2000);

	sendEvent(to_int(DeviceProperty::FrontLight), !deviceSettings.frontLight);
	sendEvent(to_int(DeviceProperty::BackLight), _flagPowerBackLight);

	DateTime now = rtc.now();

	sendEvent(to_int(DeviceProperty::DateTime), now.unixtime());

	Serial.println("🙂");
}

void loop() {
	serialEvent();
	serial2Event();
	tempSensor.update(3000);

	if (_flagPowerBackLight) {
		ledBL.setPeriod(500);
		ledPing.setPeriod(500);

		switch (deviceSettings.backLightMode) {
			default:
			case 0x1:
				{
					ledBL.setPeriod(1500);
					ledPing.setPeriod(1500);

					(void)ledBL.doubleBlink(100, 1);
					(void)ledPing.doubleBlink(100, 1);
					break;
				}
			case 0x2:
				{
					(void)ledBL.doubleBlink(80, 8);
					(void)ledPing.doubleBlink(80, 8);
					break;
				}
			/*
			case 0x2:
				{
					ledBL.blink();
					ledPing.blink();
					break;
				}
			case 0x3:
				{
					if (_flashBlink) {
						_flashBlink = !ledBL.flash(7);
						(void)ledPing.flash(7);
					} else {
						_flashBlink = ledBL.doubleBlink(50, 2);
						(void)ledPing.doubleBlink(50, 2);
					}
					break;
				}
			
			case 0x8:
				{
					//ledBL.flash(20);
					//ledPing.flash(20);
					break;
				}
			case 0x9:
				{
					//ledBL.setPeriod( 5000 );
					//ledPing.setPeriod( 5000 );

					//ledBL.blink();
					//ledPing.blink();
					break;
				}
			*/
		}
	} else if (digitalRead(PIN_POWER_BACK_LIGHT)) {
		ledBL.off();
		ledPing.off();
	}

	if (!_flagPowerBackLight) (void)ledPing.doubleBlink(50, 10);
}