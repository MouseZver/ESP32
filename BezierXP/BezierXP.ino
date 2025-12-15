#include <Arduino.h>
#include <TFT_eSPI.h>
#include "BezierAnimation.h"

TFT_eSPI tft = TFT_eSPI();
BezierAnimation bezierAnim;

void setup() {
  // Инициализируем дисплей
  tft.init();
  tft.setRotation(1); // поворот экрана (проверь нужное значение)
  tft.fillScreen(TFT_BLACK); // очищаем экран

  randomSeed(analogRead(0)); // инициализируем случайные числа
  
  bezierAnim.begin();
}

void loop() {
  bezierAnim.update();
  bezierAnim.draw();
}