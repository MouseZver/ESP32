#include "StringUtils.h"
#include <ctype.h>
#include <string.h>

// Удаляет начальные и конечные пробельные символы из C-строки
void StringUtils::trim(char *str) {
  if (str == nullptr) return;

  char *src = str;
  char *dst = str;

  // Пропускаем начальные пробелы
  while (*src != '\0' && isspace((unsigned char)*src)) {
    src++;
  }

  // Копируем содержимое без лишних пробелов
  while (*src != '\0') {
    *dst++ = *src++;
  }
  *dst = '\0';

  // Убираем пробелы с конца
  while (dst > str && isspace((unsigned char)*(dst - 1))) {
    *(--dst) = '\0';
  }
}

// Возвращает новую строку без начальных и конечных пробелов
String StringUtils::trim(const String &str) {
  int start = 0;
  int end = str.length() - 1;

  while (start <= end && isspace(str[start])) start++;
  while (end >= start && isspace(str[end])) end--;

  return str.substring(start, end + 1);
}

// Переводит C-строку в верхний регистр
void StringUtils::toUpperCase(char *str) {
  if (str == nullptr) return;
  while (*str) {
    *str = toupper((unsigned char)*str);
    str++;
  }
}

// Возвращает новую строку в верхнем регистре
String StringUtils::toUpperCase(const String &str) {
  String result = str;
  result.toUpperCase();
  return result;
}

// Переводит C-строку в нижний регистр
void StringUtils::toLowerCase(char *str) {
  if (str == nullptr) return;
  while (*str) {
    *str = tolower((unsigned char)*str);
    str++;
  }
}

// Возвращает новую строку в нижнем регистре
String StringUtils::toLowerCase(const String &str) {
  String result = str;
  result.toLowerCase();
  return result;
}

// Заменяет все вхождения oldChar на newChar в C-строке
void StringUtils::replace(char *str, char oldChar, char newChar) {
  if (str == nullptr) return;
  while (*str) {
    if (*str == oldChar) *str = newChar;
    str++;
  }
}

// Заменяет все вхождения подстроки в String
String StringUtils::replace(const String &str, const String &oldStr, const String &newStr) {
  String result = "";
  int pos = 0;
  int found;

  while ((found = str.indexOf(oldStr, pos)) != -1) {
    result += str.substring(pos, found);
    result += newStr;
    pos = found + oldStr.length();
  }

  result += str.substring(pos); // Остаток строки
  return result;
}

// Проверяет, начинается ли строка с указанного префикса
bool StringUtils::startsWith(const char *str, const char *prefix) {
  if (str == nullptr || prefix == nullptr) return false;
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

bool StringUtils::startsWith(const String &str, const String &prefix) {
  return str.startsWith(prefix);
}

// Проверяет, заканчивается ли строка указанным суффиксом
bool StringUtils::endsWith(const char *str, const char *suffix) {
  if (str == nullptr || suffix == nullptr) return false;
  size_t len = strlen(str);
  size_t suffixLen = strlen(suffix);

  if (suffixLen > len) return false;
  return strcmp(str + len - suffixLen, suffix) == 0;
}

bool StringUtils::endsWith(const String &str, const String &suffix) {
  return str.endsWith(suffix);
}

// Проверяет, содержит ли строка подстроку
bool StringUtils::contains(const char *str, const char *substr) {
  return str != nullptr && substr != nullptr && strstr(str, substr) != nullptr;
}

bool StringUtils::contains(const String &str, const String &substr) {
  return str.indexOf(substr) != -1;
}

// Добавляет слева до нужной длины указанным символом
void StringUtils::padLeft(char *str, size_t totalLength, char padChar) {
  if (str == nullptr) return;
  size_t len = strlen(str);
  if (len >= totalLength) return;

  memmove(str + (totalLength - len), str, len + 1);
  memset(str, padChar, totalLength - len);
}

String StringUtils::padLeft(const String &str, size_t totalLength, char padChar) {
  String result = str;
  while (result.length() < totalLength) {
    result = String(padChar) + result;
  }
  return result;
}

// Добавляет справа до нужной длины указанным символом
void StringUtils::padRight(char *str, size_t totalLength, char padChar) {
  size_t len = strlen(str);
  if (len >= totalLength) return;
  memset(str + len, padChar, totalLength - len);
  str[totalLength] = '\0';
}

String StringUtils::padRight(const String &str, size_t totalLength, char padChar) {
  String result = str;
  while (result.length() < totalLength) {
    result += String(padChar);
  }
  return result;
}

// Извлекает подстроку с поддержкой отрицательных индексов
String StringUtils::substring(const String &str, int start, int end) {
  size_t length = str.length();

  // Обработка отрицательных индексов
  if (start < 0) start += length;
  if (end < 0) end += length;

  // Ограничиваем диапазон
  size_t safeStart = min(static_cast<size_t>(start), length);
  size_t safeEnd = min(static_cast<size_t>(end), length);

  if (safeStart > safeEnd) return "";

  return str.substring(safeStart, safeEnd);
}

// Более низкоуровневая версия substring для C-строк
void StringUtils::substring(char *dest, const char *src, size_t start, size_t length) {
  if (!dest || !src) return;
  size_t srcLen = strlen(src);

  if (start >= srcLen) {
    dest[0] = '\0';
    return;
  }

  size_t copyLen = min(length, srcLen - start);
  strncpy(dest, src + start, copyLen);
  dest[copyLen] = '\0';
}