#include <ArduinoJson.h>   // Добавлено (предполагается, что используется)
#include <StringUtils.h>

// === Буфер для приёма ===
#define MAX_MSG_LEN 128
char incomingBuffer[MAX_MSG_LEN];
int bufferIndex = 0;  // volatile не нужен — serialEvent не в ISR
unsigned long lastByteTime = 0;

// === Обработчик команд ===
void commandsTerminal(JsonDocument& doc) {
    // Извлекаем данные
    if (!doc["name"].is<int>() || !doc["event"].is<int>()) {
        Serial.println("Error: 'name' or 'event' is not an integer");
        return;
    }

    int name = doc["name"].as<int>();
    int event = doc["event"].as<int>();
    int value = doc["value"].as<int>();  // может быть 0, если нет

    // Проверка диапазона для Button
    if (name < static_cast<int>(Button::Up) || name > static_cast<int>(Button::Enter)) {
        Serial.printf("Invalid button ID: %d\n", name);
        return;
    }
    Button button = static_cast<Button>(name);

    // Проверка диапазона для ButtonEvent (опционально)
    if (event < static_cast<int>(ButtonEvent::Press) || event > static_cast<int>(ButtonEvent::Release)) {
        // Можно пропустить или логировать
    }
    ButtonEvent buttonEvent = static_cast<ButtonEvent>(event);

    switch (button) {
        case Button::Up:
        {
            switch (buttonEvent) {
                case ButtonEvent::Click:
                {
                    deviceSettings.frontLight = !digitalRead(IN4);
                    digitalWrite(IN4, deviceSettings.frontLight);
                    sendEvent(to_int(DeviceProperty::FrontLight), !deviceSettings.frontLight);
                    break;
                }
                case ButtonEvent::MultiClick:
                {
                    if (value == 2) {
                        deviceSettings.backLightMode++;
                        if (deviceSettings.backLightMode > LEDBL_MODE_MAX) {
                            deviceSettings.backLightMode = 1;
                        }
                        sendEvent(to_int(DeviceProperty::BackLightMode), deviceSettings.backLightMode);
                    }
                    break;
                }
                case ButtonEvent::Hold:
                {
                    _flagPowerBackLight = !_flagPowerBackLight;
                    sendEvent(to_int(DeviceProperty::BackLight), _flagPowerBackLight);
                    break;
                }
                default:
                {
                    // Неизвестное событие для кнопки
                    break;
                }
            }
            break;
        }
        case Button::Down:
        {
			switch (buttonEvent) {
				case ButtonEvent::Click:
                {
					int p = digitalRead( IN2 );
					
					if ( p ) 
					{
						digitalWrite( IN2, ! p );
						sendEvent( to_int(DeviceProperty::Power), true );
						delay( value );
					}
					else
					{
						sendEvent( to_int(DeviceProperty::Power), false );
					}

					digitalWrite( IN3, ! p );

					if ( p )
					{
						sendEvent( to_int(DeviceProperty::Power), true );
					}
					else
					{
						delay( value );
						digitalWrite( IN2, 1 );
						sendEvent( to_int(DeviceProperty::Power), false );
					}
					break;
                }
				case ButtonEvent::Hold:
                {
					int n = digitalRead( NEON1 );
					
					digitalWrite( NEON1, ! n );
					digitalWrite( NEON2, ! n );
					break;
                }
				default:
                {
                    // Неизвестное событие для кнопки
                    break;
                }
			}
            break;
        }
        case Button::Enter:
        {
            if (buttonEvent == ButtonEvent::Click) {
                bool p = digitalRead(IN2);
                if (!p) {
                    digitalWrite(IN3, HIGH);
                    sendEvent(to_int(DeviceProperty::Power), false);
                    delay(1000);
                    digitalWrite(IN2, HIGH);
                    sendEvent(to_int(DeviceProperty::Power), false);
                }
                saveSettings(deviceSettings);
                digitalWrite(IN1, HIGH);  // сигнализация
            }
            break;
        }
        default:
            Serial.println("Unknown button (should not happen)");
            break;
    }
}

// ==== Функция обработки команд ====
void processCommand(const char* jsonStr) {
    String str = jsonStr;
    str = StringUtils::trim(str);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, str);
    Serial.println("GET:");
    Serial.println(str);

    if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.f_str());
        return;
    }

    commandsTerminal(doc);
}

// ==== Приём данных по Serial2 ====
void serialEvent() {
    while (Serial2.available()) {
        char c = Serial2.read();

        if (bufferIndex < MAX_MSG_LEN - 1) {
            incomingBuffer[bufferIndex++] = c;
            incomingBuffer[bufferIndex] = '\0';  // завершаем строку

            if (c == '\n') {
                processCommand(incomingBuffer);
                bufferIndex = 0;
                incomingBuffer[0] = '\0';
            }

            lastByteTime = millis();
        } else {
            // Буфер переполнен
            Serial.println("Buffer overflow!");
            bufferIndex = 0;
            incomingBuffer[0] = '\0';
            lastByteTime = millis();
        }
    }

    // Таймаут незавершённой команды
    if (bufferIndex > 0 && millis() - lastByteTime > 1000) {
        incomingBuffer[bufferIndex] = '\0';
        processCommand(incomingBuffer);
        bufferIndex = 0;
        incomingBuffer[0] = '\0';
    }
}