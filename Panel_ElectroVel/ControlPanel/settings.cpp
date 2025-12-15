#include "settings.h"
#include <math.h>

// ===============================
// == Параметры анимации ==
// ===============================

// Скорость движения точек Безье
// 1.0 — нормальная скорость, >1 быстрее, <1 медленнее
float speedFactor = 1.0f;

// Интенсивность хаоса (случайных изменений направления)
// 1.0 — максимум хаоса, 0 — стабильное движение
float chaosFactor = 1.0f;

// Насколько быстро меняется цветовая гамма
// 0.1 — медленно, 1.0 — очень быстро
float hueShift = 0.3f;

// Цветовой тон начального оттенка (от 0 до 360 градусов)
float hue = 0.0f;

void hsvToRgb(float h, uint8_t s, uint8_t v, uint8_t rgb[3]) {
  int i = int(h / 60.0f);
  float f = h / 60.0f - i;
  int p = v * (255 - s) / 255;
  int q = v * (255 - s * f) / 255;
  int t = v * (255 - s * (1 - f)) / 255;

  switch (i) {
    case 0: rgb[0] = v; rgb[1] = t; rgb[2] = p; break;
    case 1: rgb[0] = q; rgb[1] = v; rgb[2] = p; break;
    case 2: rgb[0] = p; rgb[1] = v; rgb[2] = t; break;
    case 3: rgb[0] = p; rgb[1] = q; rgb[2] = v; break;
    case 4: rgb[0] = t; rgb[1] = p; rgb[2] = v; break;
    default: rgb[0] = v; rgb[1] = p; rgb[2] = q; break;
  }
}

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}