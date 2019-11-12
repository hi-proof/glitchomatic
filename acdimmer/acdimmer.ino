#include <FastLED.h>

#define PIN_ZC          D4
#define PIN_GLITCH      D1
#define PIN_BRIGHTNESS  D2
#define PIN_POT         A0

#define PINS_COUNT (4)
uint8_t pins[PINS_COUNT] = {
  D5,
  D6,
  D7,
  D8
};

volatile bool off = false;
volatile uint32_t ticks_counter = 0;
uint8_t brightnesses[PINS_COUNT] = {0};
uint8_t target_brightness;
uint8_t g_glitch = 0;
uint8_t g_brightness = 0;

void zc_isr()
{
  timer1_disable();
  ticks_counter = 0;

  timer1_write(155); // div16 is 5mhz or 0.2uS per tick. 155 * 0.2 = 31uS so that 256 of those can be ~8000
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
}

void ICACHE_RAM_ATTR timer_isr()
{
  ticks_counter++;

  if (off) {
    return;
  }

  for (int i=0; i<PINS_COUNT; i++) {
    // add 10 "ticks" for about 300 uS buffer between the zero crossing and the 0-8000 uS delay
    int trigger = 255 - brightnesses[i] + 10;
    if (ticks_counter == trigger) {
      digitalWrite(pins[i], HIGH);
    } else if (ticks_counter == trigger + 1) {
      digitalWrite(pins[i], LOW);
    }
  }
}

void setup() {
  // timer isr
  timer1_attachInterrupt(timer_isr);

  // gpio
  for (int i=0; i<PINS_COUNT; i++) {
    pinMode(pins[i], OUTPUT);
  }
  pinMode(PIN_ZC, INPUT_PULLUP);

  // zc interrupt - do this last
  attachInterrupt(digitalPinToInterrupt(PIN_ZC), zc_isr, RISING);
}

void loop() {
  // read pots
  digitalWrite(PIN_GLITCH, HIGH);
  g_glitch = analogRead(PIN_POT) << 2;
  digitalWrite(PIN_GLITCH, LOW);
  
  digitalWrite(PIN_BRIGHTNESS, HIGH);
  g_brightness = analogRead(PIN_POT) << 2;
  digitalWrite(PIN_BRIGHTNESS, LOW);


  // update brightness levels based on pots

  // keep the lights off maybe
  off = (g_brightness < 5);
  
  // we have 4 ranges for the "glitch" factor
  // off - no glitch, just keep the lights on
  // low - periodic glitch, adjust time between glitch periods and lerp speed
  // high - constant glitch, adjust noise function speed

  // off
  if (g_glitch < 10) {
    brightnesses[i] = lerp8by8(brightnesses[i], g_brightness, lerp_speed);
  // low
  } else if (g_glitch < 170) {
    for (int i=0; i<NUM_PINS; i++) {
      uint8_t rand_factor = 5 .. 20;
      uint8_t lerp_speed = 1 .. 5;
      if (random8() < factor) {
        brightnesses[i] = random8(0, 50);
      } else {
        brightnesses[i] = lerp8by8(brightnesses[i], g_brightness, lerp_speed);
      }
  // high
  } else {
    for (int i=0; i<NUM_PINS; i++) {
      float factor = 3x .. 0.3;
      brightnesses[i] = scale8(inoise8(millis() * factor), g_brightness);
    }
  }

//  //brightness = (millis() % 1000) / 4; 
//  //brightness = map(millis() % 2000, 0, 2000, 10, 240);
//  //brightness = glitch[millis() % sizeof(glitch)];//beatsin8(30);
//  brightnesses[0] = inoise8(millis() / 2);
}
