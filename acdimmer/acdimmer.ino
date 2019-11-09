#include <FastLED.h>

#define PIN_ZC          D3
#define PIN_BRIGHTNESS
#define PIN_GLITCH
#define PIN_POT

#define PINS_COUNT (4)
uint8_t pins[PINS_COUNT] = {
  D5,
  D6,
  D7,
  D8
};

volatile uint32_t ticks_counter = 0;
uint8_t brightnesses[PINS_COUNT] = {0};

void zc_isr()
{
  ticks_counter = 0;
  
  done = false;
  donedone = false; 

  timer1_disable();
  timer1_write(155); // div16 is 5mhz or 0.2uS per tick. 155 * 0.2 = 31uS so that 256 of those can be ~8000
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
}

void ICACHE_RAM_ATTR timer_isr()
{
  ticks_counter++;

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
  // update brightness levels based on pots
  //brightness = (millis() % 1000) / 4; 
  //brightness = map(millis() % 2000, 0, 2000, 10, 240);
  //brightness = glitch[millis() % sizeof(glitch)];//beatsin8(30);
  brightnesses[0] = inoise8(millis() / 2);
}
