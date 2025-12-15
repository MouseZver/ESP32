#ifndef EVENT_UTILS_H
#define EVENT_UTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <SoftwareSerial.h>

//extern SoftwareSerial Serial2;

// Прототип обобщённой функции sendEvent
template <typename TName, typename TValue, typename TEvent>
void sendEvent(TName name, TValue value, TEvent event);

// Перегрузка для вызова без event
template <typename TName, typename TValue>
void sendEvent(TName name, TValue value);

// --- Шаблонная функция sendEvent (в заголовочном файле!) ---
template <typename TName, typename TValue, typename TEvent>
void sendEvent(TName name, TValue value, TEvent event) {
  static JsonDocument doc;
  static String jsonStr;
  doc.clear();

  doc["name"] = name;
  doc["value"] = value;
  doc["event"] = event;

  serializeJson(doc, jsonStr);
  Serial2.println(jsonStr);

  Serial.println("Посылаю:");
  Serial.println(jsonStr);
}

// Перегрузка для вызова без event
template <typename TName, typename TValue>
void sendEvent(TName name, TValue value) {
  static JsonDocument doc;
  static String jsonStr;
  doc.clear();

  doc["name"] = name;
  doc["value"] = value;

  serializeJson(doc, jsonStr);
  Serial2.println(jsonStr);

  Serial.println("Посылаю:");
  Serial.println(jsonStr);
}

#endif