#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// === Настройки дисплея ===
#define WIDTH  240
#define HEIGHT 240
#define FB_SIZE (WIDTH * HEIGHT)

// === Параметры анимации ===
#define NUM_CHAINS         3           // количество кривых (цепочек)
#define POINTS_PER_CHAIN   12          // должно быть кратно 3 + 1 для Безье
#define SEGMENTS_PER_CHAIN 3          // количество сегментов Безье (зависит от POINTS_PER_CHAIN)
#define POINT_DENSITY      15          // точек на сегмент Безье (чем больше — тем плотнее линия)

// === Режим отрисовки ===
#define MODE_POINTS        0
#define MODE_LINES         1
#define DRAW_MODE          MODE_POINTS  // используем ТОЧКИ

// === Прочие параметры ===
#define SMOOTHNESS         0.8f        // сила контрольных точек для замыкания
#define CHAOS_FACTOR       0.5f        // хаос движения
#define SPEED_FACTOR       1.0f        // скорость анимации
#define HUE_SHIFT          0.5f        // смещение оттенка за кадр

#endif