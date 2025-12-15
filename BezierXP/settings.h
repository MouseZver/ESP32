#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// === Настройки дисплея ===
#define WIDTH  240
#define HEIGHT 240
#define FB_SIZE (WIDTH * HEIGHT)

// === Параметры анимации ===
#define NUM_CHAINS         3
#define POINTS_PER_CHAIN   12
#define SEGMENTS_PER_CHAIN 3
#define POINT_DENSITY      15

// === Режим отрисовки ===
#define MODE_POINTS        0
#define MODE_LINES         1
#define DRAW_MODE          MODE_POINTS

// === ФИЗИЧЕСКИЕ ПАРАМЕТРЫ АНИМАЦИИ ===
#define SMOOTHNESS         0.8f
#define CHAOS_FACTOR       0.5f
#define SPEED_FACTOR       1.0f
#define HUE_SHIFT          0.5f

#endif