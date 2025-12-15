// Макрос swap с безопасным именем
#define swapVals(a, b) { auto t = a; a = b; b = t; }

#include "BezierAnimation.h"
#include "settings.h"
#include <math.h>

// Глобальный объект дисплея (должен быть объявлен в основном скетче)
extern TFT_eSPI tft;

BezierAnimation::BezierAnimation() {}

void BezierAnimation::begin() {
    frameBuffer = (uint16_t*)malloc(FB_SIZE * sizeof(uint16_t));
    if (!frameBuffer) while (1); // остановка при нехватке памяти

    for (int c = 0; c < NUM_CHAINS; c++) {
        int gridCols = ceil(sqrt(NUM_CHAINS));
        int row = c / gridCols;
        int col = c % gridCols;

        int centerX = (WIDTH / gridCols) * col + (WIDTH / gridCols) / 2;
        int centerY = (HEIGHT / gridCols) * row + (HEIGHT / gridCols) / 2;

        for (int i = 0; i < POINTS_PER_CHAIN; i++) {
            float angle = (360.0f / POINTS_PER_CHAIN) * i;
            float rad = angle * 0.0174532925f;
            float radius = random(30, 80);

            this->points[c][i].x = centerX + cos(rad) * radius;
            this->points[c][i].y = centerY + sin(rad) * radius;
            this->points[c][i].dx = random(-100, 101) / 100.0f;
            this->points[c][i].dy = random(-100, 101) / 100.0f;
        }
    }

    for (int i = 0; i < FB_SIZE; i++) {
        frameBuffer[i] = TFT_BLACK;
    }
}

void BezierAnimation::update() {
    static unsigned long lastTime = 0;
    float dt = (millis() - lastTime) / 1000.0f;
    lastTime = millis();

    // Сохраняем текущие позиции как предыдущие
    for (int c = 0; c < NUM_CHAINS; c++) {
        for (int i = 0; i < POINTS_PER_CHAIN; i++) {
            Point* p = &this->points[c][i];
            this->prevPoints[c][i] = *p; // Копируем текущую позицию
        }
    }

    for (int c = 0; c < NUM_CHAINS; c++) {
        for (int i = 0; i < POINTS_PER_CHAIN; i++) {
            Point* p = &this->points[c][i];

            float chaosX = (random(-100, 101) / 1000.0f) * CHAOS_FACTOR;
            float chaosY = (random(-100, 101) / 1000.0f) * CHAOS_FACTOR;

            p->x += (p->dx + chaosX) * dt * 60 * SPEED_FACTOR;
            p->y += (p->dy + chaosY) * dt * 60 * SPEED_FACTOR;

            if (p->x < 0 || p->x >= WIDTH) {
                p->x = constrain(p->x, 0, WIDTH - 1);
                p->dx = -p->dx;
            }

            if (p->y < 0 || p->y >= HEIGHT) {
                p->y = constrain(p->y, 0, HEIGHT - 1);
                p->dy = -p->dy;
            }
        }
    }
}

void BezierAnimation::draw() {
    uint8_t rgb[3];
    hsvToRgb(hue, 255, 0.6f, rgb); // Value = 0.6 для тёмного тона
    uint16_t color = rgb565(rgb[0], rgb[1], rgb[2]);

    for (int c = 0; c < NUM_CHAINS; c++) {
        drawBezierChainFB(this->points[c], color, SEGMENTS_PER_CHAIN);
    }

    tft.pushColors(frameBuffer, FB_SIZE, true);
}

void BezierAnimation::fadeFrameBuffer(float fadeFactor) {
    for (int i = 0; i < FB_SIZE; i++) {
        uint16_t c = frameBuffer[i];
        if (c == TFT_BLACK) continue;

        uint8_t r = ((c >> 8) & 0xF8);
        uint8_t g = ((c >> 3) & 0xFC);
        uint8_t b = ((c << 3) & 0xE0);

        // Экспоненциальное затухание
        r = max(0, (int)(r * fadeFactor));
        g = max(0, (int)(g * fadeFactor));
        b = max(0, (int)(b * fadeFactor));

        // Обновляем пиксель
        frameBuffer[i] = rgb565(r, g, b);
    }
}

void BezierAnimation::drawLineFB(int x0, int y0, int x1, int y1, uint16_t color) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { swapVals(x0, y0); swapVals(x1, y1); }
    if (x0 > x1) { swapVals(x0, x1); swapVals(y0, y1); }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int err = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++) {
        int y = y0;
        if (steep) swapVals(x0, y);
        if (x0 >= 0 && x0 < WIDTH && y >= 0 && y < HEIGHT)
            frameBuffer[y * WIDTH + x0] = color;
        if (steep) swapVals(x0, y);
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void BezierAnimation::bezierPoint(float t, Point* p0, Point* p1, Point* p2, Point* p3, float* x, float* y) {
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    *x = uuu * p0->x + 3 * uu * t * p1->x + 3 * u * tt * p2->x + ttt * p3->x;
    *y = uuu * p0->y + 3 * uu * t * p1->y + 3 * u * tt * p2->y + ttt * p3->y;
}

void BezierAnimation::drawBezierChainFB(Point* chain, uint16_t color, int segments) {
    // Основные сегменты Безье
    for (int s = 0; s < segments; s++) {
        int base = s * 3;
        if (base + 3 >= POINTS_PER_CHAIN) break;

        Point* p0 = &chain[base];
        Point* p1 = &chain[base + 1];
        Point* p2 = &chain[base + 2];
        Point* p3 = &chain[base + 3];

        for (int step = 1; step <= POINT_DENSITY; step++) {
            float t = step / (float)POINT_DENSITY;
            float x, y;
            bezierPoint(t, p0, p1, p2, p3, &x, &y);

            x = constrain(x, 0, WIDTH - 1);
            y = constrain(y, 0, HEIGHT - 1);

            frameBuffer[int(y) * WIDTH + int(x)] = color;

            // Рисуем след
            drawTrail(x, y, color, 0.95f); // 0.95f — коэффициент затухания
        }
    }

    // Замыкание: соединяем последнюю точку с первой
    Point* p0 = &chain[0];
    Point* p3 = &chain[POINTS_PER_CHAIN - 1];

    float dx1 = p0->x - chain[1].x;
    float dy1 = p0->y - chain[1].y;
    float dx2 = p3->x - chain[POINTS_PER_CHAIN - 2].x;
    float dy2 = p3->y - chain[POINTS_PER_CHAIN - 2].y;

    Point p1, p2;
    p1.x = p0->x + dx1 * SMOOTHNESS;
    p1.y = p0->y + dy1 * SMOOTHNESS;
    p2.x = p3->x + dx2 * SMOOTHNESS;
    p2.y = p3->y + dy2 * SMOOTHNESS;

    p1.x = constrain(p1.x, 0, WIDTH - 1);
    p1.y = constrain(p1.y, 0, HEIGHT - 1);
    p2.x = constrain(p2.x, 0, WIDTH - 1);
    p2.y = constrain(p2.y, 0, HEIGHT - 1);

    for (int step = 1; step <= POINT_DENSITY; step++) {
        float t = step / (float)POINT_DENSITY;
        float x, y;
        bezierPoint(t, p0, &p1, &p2, p3, &x, &y);

        x = constrain(x, 0, WIDTH - 1);
        y = constrain(y, 0, HEIGHT - 1);

        frameBuffer[int(y) * WIDTH + int(x)] = color;

        // Рисуем след
        drawTrail(x, y, color, 0.95f);
    }
}

void BezierAnimation::drawTrail(float x, float y, uint16_t color, float fadeFactor) {
    // Рисуем предыдущие точки с уменьшающейся яркостью
    for (int i = 1; i <= 10; i++) { // Максимум 10 точек в следе
        float trailX = x + random(-1, 2); // Легкое смещение для размытия
        float trailY = y + random(-1, 2);

        trailX = constrain(trailX, 0, WIDTH - 1);
        trailY = constrain(trailY, 0, HEIGHT - 1);

        uint8_t r = ((color >> 8) & 0xF8);
        uint8_t g = ((color >> 3) & 0xFC);
        uint8_t b = ((color << 3) & 0xE0);

        // Уменьшаем яркость
        r = max(0, (int)(r * pow(fadeFactor, i)));
        g = max(0, (int)(g * pow(fadeFactor, i)));
        b = max(0, (int)(b * pow(fadeFactor, i)));

        uint16_t trailColor = rgb565(r, g, b);
        frameBuffer[int(trailY) * WIDTH + int(trailX)] = trailColor;
    }
}

// Конвертация HSV → RGB
void BezierAnimation::hsvToRgb(float h, float s, float v, uint8_t rgb[3]) {
    h = fmod(h, 360.0f); // Ограничиваем оттенок до [0, 360)
    if (h < 0) h += 360;
    s = constrain(s, 0.0f, 1.0f);
    v = constrain(v, 0.0f, 1.0f);

    float c = v * s;          // Интенсивность
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1)); // Вторичная интенсивность
    float m = v - c;          // Минимальное значение

    float r, g, b;

    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    rgb[0] = (r + m) * 255; // Красный
    rgb[1] = (g + m) * 255; // Зелёный
    rgb[2] = (b + m) * 255; // Синий
}

// Конвертация RGB → RGB565
uint16_t BezierAnimation::rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}