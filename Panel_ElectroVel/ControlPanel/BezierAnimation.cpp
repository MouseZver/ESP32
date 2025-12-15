// Макрос swap с безопасным именем
#define swapVals(a, b) { auto t = a; a = b; b = t; }

#include "BezierAnimation.h"
#include "settings.h"
#include <math.h>

BezierAnimation::BezierAnimation() {}

void BezierAnimation::begin() {
  frameBuffer = (uint16_t*)malloc(FB_SIZE * sizeof(uint16_t));
  if (!frameBuffer) while (1); // остановка при нехватке памяти

  for (int c = 0; c < NUM_CHAINS; c++) {
    int gridCols = ceil(sqrt(NUM_CHAINS)); // количество колонок в сетке
    int row = c / gridCols;
    int col = c % gridCols;

    int centerX = (WIDTH / gridCols) * col + (WIDTH / gridCols) / 2;
    int centerY = (HEIGHT / gridCols) * row + (HEIGHT / gridCols) / 2;

    for (int i = 0; i < POINTS_PER_CHAIN; i++) {
      float angle = (360.0f / POINTS_PER_CHAIN) * i;
      float rad = angle * 0.0174532925f;
      float radius = random(20, 50);

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

  fadeFrameBuffer(4);

  for (int c = 0; c < NUM_CHAINS; c++) {
    for (int i = 0; i < POINTS_PER_CHAIN; i++) {
      Point* p = &this->points[c][i];

      float chaosX = (random(-100, 101) / 1000.0f) * chaosFactor;
      float chaosY = (random(-100, 101) / 1000.0f) * chaosFactor;

      p->x += (p->dx + chaosX) * dt * 60 * speedFactor;
      p->y += (p->dy + chaosY) * dt * 60 * speedFactor;

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

  hue += hueShift;
  if (hue >= 360.0f) hue -= 360.0f;
}

void BezierAnimation::draw() {
  for (int c = 0; c < NUM_CHAINS; c++) {
    uint8_t rgb[3];
    hsvToRgb(hue + c * (360.0f / NUM_CHAINS), 255, 255, rgb);
    uint16_t color = rgb565(rgb[0], rgb[1], rgb[2]);

    drawBezierChainFB(this->points[c], color, SEGMENTS_PER_CHAIN);
  }

  tft.pushColors(frameBuffer, FB_SIZE, true);
}

void BezierAnimation::fadeFrameBuffer(uint8_t strength) {
  for (int i = 0; i < FB_SIZE; i++) {
    uint16_t c = frameBuffer[i];
    uint8_t r = ((c >> 8) & 0xF8);
    uint8_t g = ((c >> 3) & 0xFC);
    uint8_t b = ((c << 3) & 0xE0);

    r = max(0, r - strength);
    g = max(0, g - strength);
    b = max(0, b - strength);

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
  // Отрисовка всех участков Безье
  for (int s = 0; s < segments; s++) {
    int base = s * 3;
    if (base + 3 >= POINTS_PER_CHAIN) break;

    Point* p0 = &chain[base];
    Point* p1 = &chain[base + 1];
    Point* p2 = &chain[base + 2];
    Point* p3 = &chain[base + 3];

    float lastX = p0->x;
    float lastY = p0->y;

    for (int step = 1; step <= POINT_DENSITY; step++) {
      float t = step / (float)POINT_DENSITY;
      float x, y;
      bezierPoint(t, p0, p1, p2, p3, &x, &y);

      // Ограничиваем координаты
      x = constrain(x, 0, WIDTH - 1);
      y = constrain(y, 0, HEIGHT - 1);

      if (DRAW_MODE == MODE_LINES) {
        drawLineFB(lastX, lastY, x, y, color);
      } else if (DRAW_MODE == MODE_POINTS) {
        frameBuffer[int(y) * WIDTH + int(x)] = color;
      }

      lastX = x;
      lastY = y;
    }
  }

  // ==== Гладкое замыкание цепочки Безье ====
  Point* p0 = &chain[0];
  Point* p3 = &chain[POINTS_PER_CHAIN - 1];

  // Вычисляем контрольные точки так, чтобы кривая не вылетала за экран
  Point p1, p2;

  // Берём соседние точки для гладкости
  float dx1 = p0->x - chain[1].x;
  float dy1 = p0->y - chain[1].y;
  float dx2 = p3->x - chain[POINTS_PER_CHAIN - 2].x;
  float dy2 = p3->y - chain[POINTS_PER_CHAIN - 2].y;

  // Усиление контрольных точек
  float controlStrength = SMOOTHNESS;

  p1.x = p0->x + dx1 * controlStrength;
  p1.y = p0->y + dy1 * controlStrength;

  p2.x = p3->x + dx2 * controlStrength;
  p2.y = p3->y + dy2 * controlStrength;

  // Ограничиваем контрольные точки, чтобы кривая не вылетала за экран
  p1.x = constrain(p1.x, 0, WIDTH - 1);
  p1.y = constrain(p1.y, 0, HEIGHT - 1);
  p2.x = constrain(p2.x, 0, WIDTH - 1);
  p2.y = constrain(p2.y, 0, HEIGHT - 1);

  float lastX = p0->x;
  float lastY = p0->y;

  for (int step = 1; step <= POINT_DENSITY; step++) {
    float t = step / (float)POINT_DENSITY;
    float x, y;
    bezierPoint(t, p0, &p1, &p2, p3, &x, &y);

    // Ограничиваем координаты
    x = constrain(x, 0, WIDTH - 1);
    y = constrain(y, 0, HEIGHT - 1);

    if (DRAW_MODE == MODE_LINES) {
      drawLineFB(lastX, lastY, x, y, color);
    } else if (DRAW_MODE == MODE_POINTS) {
      frameBuffer[int(y) * WIDTH + int(x)] = color;
    }

    lastX = x;
    lastY = y;
  }
}