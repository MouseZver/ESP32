#include <StringUtils.h>

// === Буфер для приёма ===
#define MAX_MSG_LEN 128
char incomingBuffer[MAX_MSG_LEN];
volatile int bufferIndex = 0;
volatile unsigned long lastByteTime = 0;

void commandsTerminal( JsonDocument doc )
{
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
        //modeBackLamp
        break;
    case 4:
        display.setTime(doc["value"].as<uint32_t>());
        break;
    case 5:
        display.setTemperature(doc["value"].as<float>());
        break;
    default:
        // Необязательно: обработка неизвестных значений
        break;
  }
}

// ==== Функция обработки команд ====
void processCommand(const char* jsonStr) {
  String str = jsonStr;
  str = StringUtils::trim(str);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, str);
	Serial.println("Пришло:");
  Serial.println(str);

  if (error) {
    Serial.println("Ошибка разбора JSON");
    return;
  }

	//Serial.println("Ответ успешно получен");
  commandsTerminal( doc );
}

// ==== Приём данных по Serial2 ====
void serialEvent() {
  while (Serial2.available()) {
    char c = Serial2.read();

    // Если есть место в буфере
    if (bufferIndex < MAX_MSG_LEN - 1) {
      incomingBuffer[bufferIndex] = c;        // сохраняем символ
      noInterrupts();  // Отключаем прерывания
      uint8_t current = bufferIndex;
      current++;
      bufferIndex = current;
      interrupts();    // Включаем обратно
      incomingBuffer[bufferIndex] = '\0';       // завершаем строку

      // Если достигли конца команды
      if (c == '\n') {
        processCommand(incomingBuffer);         // обрабатываем команду
        
        bufferIndex = 0;                        // сбрасываем индекс
        incomingBuffer[0] = '\0';               // очищаем буфер
      }

      lastByteTime = millis();                  // обновляем время последнего байта
    } else {
      // Буфер переполнен — сбросим его
      bufferIndex = 0;
      incomingBuffer[0] = '\0';
      lastByteTime = millis();
    }
  }

  // Если прошло более 1 секунды после получения последнего байта и буфер не пуст
  if (bufferIndex > 0 && millis() - lastByteTime > 1000) {
    incomingBuffer[bufferIndex] = '\0';         // явно завершаем строку

    processCommand(incomingBuffer);

    bufferIndex = 0;
    incomingBuffer[0] = '\0';
  }
}
