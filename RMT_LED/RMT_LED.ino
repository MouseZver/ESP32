// Настройки
#define BRIGHTNESS_MAX 20    // Максимальная яркость (0-255)
#define FADE_DELAY     10      // Задержка между шагами затухания в миллисекундах

#define RGB_LED_GPIO PIN_RGB_LED // PIN_RGB_LED 48

// Перечисление состояний анимации
enum AnimationState {
  STATE_RED_TO_GREEN,
  STATE_GREEN_TO_BLUE,
  STATE_BLUE_TO_WHITE,
  STATE_WHITE_TO_RED
};

AnimationState currentAnimation = STATE_WHITE_TO_RED;

unsigned long lastStepTime = 0;
int fadeStep = 0;

void setup() {
  Serial.begin(115200);
  delay(500);  // Даем время на инициализацию порта
  

  setCpuFrequencyMhz(240); // Устанавливаем частоту процессора

  rmtInit(RGB_LED_GPIO, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000); // Инициализируем RMT
}

// Плавное изменение яркости от start к end
void fadeColor(uint8_t startR, uint8_t startG, uint8_t startB,
               uint8_t endR, uint8_t endG, uint8_t endB,
               uint16_t steps = 100, uint16_t delayMs = 10) {
  if (fadeStep <= steps) {
    float progress = fadeStep / (float)steps;
    uint8_t r = startR + (endR - startR) * progress;
    uint8_t g = startG + (endG - startG) * progress;
    uint8_t b = startB + (endB - startB) * progress;

    rgbLedWrite(RGB_LED_GPIO, r, g, b);
    fadeStep++;

    // Не меняем состояние до завершения перехода
  } else {
    // Переход завершён — меняем состояние
    currentAnimation = (AnimationState)((currentAnimation + 1) % 4);
    fadeStep = 0; // Сбрасываем счётчик
  }
}

void rgbLed() {
  // Время прошло достаточно?
  if (millis() - lastStepTime > FADE_DELAY) {
    lastStepTime = millis(); // Обновляем метку времени

    switch (currentAnimation) {
      case STATE_RED_TO_GREEN:
        fadeColor(BRIGHTNESS_MAX, 0, 0, 0, BRIGHTNESS_MAX, 0, 100, FADE_DELAY);
        break;
      case STATE_GREEN_TO_BLUE:
        fadeColor(0, BRIGHTNESS_MAX, 0, 0, 0, BRIGHTNESS_MAX, 100, FADE_DELAY);
        break;
      case STATE_BLUE_TO_WHITE:
        fadeColor(0, 0, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX, 100, FADE_DELAY);
        break;
      case STATE_WHITE_TO_RED:
        fadeColor(BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX, BRIGHTNESS_MAX, 0, 0, 100, FADE_DELAY);
        break;
    }
  }
}

void loop() {
  rgbLed();

  // Здесь можно выполнять любой другой код.
}