#include "AsyncTempSensor.h"

AsyncTempSensor::AsyncTempSensor(uint8_t pin, TempCallback callback)
    : _oneWire(pin), _pin(pin), _callback(callback) {}

void AsyncTempSensor::begin() {
    //if (_debug) Serial.println("AsyncTempSensor: Начинаем инициализацию...");

    findDevices();

    if (_deviceCount == 0) {
        //if (_debug) Serial.println("AsyncTempSensor: Нет датчиков");
        if (_callback) _callback(-1, NAN);
    } else {
        if (_debug) {
            //Serial.print("AsyncTempSensor: Найдено ");
            //Serial.print(_deviceCount);
            //Serial.println(" датчиков");
        }
        startConversion();
    }
}

void AsyncTempSensor::update(unsigned long interval) {
    if (_deviceCount == 0) return;

    if (_conversionStarted) {
        if (millis() - _lastConversionTime >= 750) {
            for (int i = 0; i < _deviceCount; i++) {
                _oneWire.reset();
                _oneWire.select(_addresses[i]);
                _oneWire.write(0xBE); // Чтение Scratchpad

                uint8_t data[9];
                for (int j = 0; j < 9; j++) {
                    data[j] = _oneWire.read();
                }

                int16_t raw = (int16_t)data[1] << 8 | data[0];
                float temperature = raw / 16.0;

                if (_callback) {
                    _callback(i, temperature);
                }
            }
            _conversionStarted = false;
        }
        return;
    }

    if (millis() - _lastConversionTime >= interval) {
        startConversion();
    }
}

void AsyncTempSensor::setDebugOutput(bool enabled) {
    _debug = enabled;
}

void AsyncTempSensor::findDevices() {
    _deviceCount = 0;
    _oneWire.reset();

    uint8_t address[8];
    while (_oneWire.search(address)) {
        if (OneWire::crc8(address, 7) == address[7]) {
            if (address[0] == 0x28) { // Проверка на DS18B20
                for (int i = 0; i < 8; i++) {
                    _addresses[_deviceCount][i] = address[i];
                }
                _deviceCount++;
            }
        }
    }
}

void AsyncTempSensor::startConversion() {
    if (_deviceCount == 0) return;

    _oneWire.reset();
    _oneWire.skip();
    _oneWire.write(0x44, 1); // Запуск измерения температуры

    _lastConversionTime = millis();
    _conversionStarted = true;

    //if (_debug) Serial.println("AsyncTempSensor: Запущено измерение");
}