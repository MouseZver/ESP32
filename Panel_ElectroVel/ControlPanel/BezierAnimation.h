#pragma once

#include <Arduino.h>
#include <stdint.h> // Для uint8_t, uint16_t
#include "settings.h"
#include <TFT_eSPI.h>


extern TFT_eSPI tft;

class BezierAnimation {
public:
  BezierAnimation();
  void begin();             // инициализация
  void update();            // обновление состояния
  void draw();              // отрисовка

private:
  struct Point {
    float x;
    float y;
    float dx;
    float dy;
  };

  Point points[NUM_CHAINS][POINTS_PER_CHAIN];
  uint16_t* frameBuffer = nullptr;
  const int FB_SIZE = WIDTH * HEIGHT;

  void fadeFrameBuffer(uint8_t strength);
  void drawLineFB(int x0, int y0, int x1, int y1, uint16_t color);
  void bezierPoint(float t, Point* p0, Point* p1, Point* p2, Point* p3, float* x, float* y);
  void drawBezierChainFB(Point* chain, uint16_t color, int segments);
};