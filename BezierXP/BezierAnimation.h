#ifndef BEZIER_ANIMATION_H
#define BEZIER_ANIMATION_H

#include "settings.h"
#include <TFT_eSPI.h> // Убедитесь, что библиотека подключена

extern TFT_eSPI tft;

struct Point {
    float x, y;
    float dx, dy;
};

class BezierAnimation {
public:
    BezierAnimation();
    void begin();
    void update();
    void draw();

private:
    private:
    uint16_t* frameBuffer;
    Point points[NUM_CHAINS][POINTS_PER_CHAIN];
    Point prevPoints[NUM_CHAINS][POINTS_PER_CHAIN]; // Для хранения предыдущих позиций
    float hue = 190.0f; // Фиксированный бирюзовый цвет

    void fadeFrameBuffer(float fadeFactor);
    void drawLineFB(int x0, int y0, int x1, int y1, uint16_t color);
    void bezierPoint(float t, Point* p0, Point* p1, Point* p2, Point* p3, float* x, float* y);
    void drawBezierChainFB(Point* chain, uint16_t color, int segments);
    void drawTrail(float x, float y, uint16_t color, float fadeFactor);

    // Вспомогательные функции
    void hsvToRgb(float h, float s, float v, uint8_t rgb[3]);
    uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
};

#endif