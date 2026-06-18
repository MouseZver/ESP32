// StringUtils.h

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>

class StringUtils {
public:
  // Удаление пробелов в начале и конце
  static void trim(char *str);
  static String trim(const String &str);

  // Перевод в верхний регистр
  static void toUpperCase(char *str);
  static String toUpperCase(const String &str);

  // Перевод в нижний регистр
  static void toLowerCase(char *str);
  static String toLowerCase(const String &str);

  // Замена символов
  static void replace(char *str, char oldChar, char newChar);
  static String replace(const String &str, const String &oldStr, const String &newStr);

  // Проверка начала строки
  static bool startsWith(const char *str, const char *prefix);
  static bool startsWith(const String &str, const String &prefix);

  // Проверка конца строки
  static bool endsWith(const char *str, const char *suffix);
  static bool endsWith(const String &str, const String &suffix);

  // Содержит ли строка подстроку
  static bool contains(const char *str, const char *substr);
  static bool contains(const String &str, const String &substr);

  // Добавление слева
  static void padLeft(char *str, size_t totalLength, char padChar);
  static String padLeft(const String &str, size_t totalLength, char padChar);

  // Добавление справа
  static void padRight(char *str, size_t totalLength, char padChar);
  static String padRight(const String &str, size_t totalLength, char padChar);

  // Извлечение подстроки
  static String substring(const String &str, int start, int end);
  static void substring(char *dest, const char *src, size_t start, size_t length);
};

#endif