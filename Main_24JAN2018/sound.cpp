
#include "sound.h"

#include <Arduino.h>

const int spk[2] = {3, 4};

void init_sound() {
  pinMode(spk[0], OUTPUT);
  pinMode(spk[1], OUTPUT);
  digitalWrite(spk[0], LOW);
  digitalWrite(spk[1], LOW);
}

void chip_tone(unsigned long for_millis, unsigned long period_micros, int volume) {
  const uint16_t period_cycles = ((((unsigned long)F_CPU) / 1000000L) / 4) * period_micros;
  const uint16_t half_delay_cycles = period_cycles / 2;
  const uint16_t play_iters = (for_millis * 1000L) / period_micros;

  // Off: don't burn speaker
  OCR0A = 0;
  OCR0B = 0;

  noInterrupts();
  const auto  ra_save = TCCR0A;
  const auto  rb_save = TCCR0B;
  TCCR0A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR0B = _BV(CS22);

  TCCR0B = (TCCR0B & 0b11111000) | 0x01;

  for (uint16_t i = 0; i < play_iters; ++i) {
    OCR0B = 0;
    OCR0A = volume;
    _delay_loop_2(half_delay_cycles);

    OCR0A = 0;
    OCR0B = volume;
    _delay_loop_2(half_delay_cycles);
  }

  OCR0A = 0;
  OCR0B = 0;

  TCCR0A = ra_save;
  TCCR0B = rb_save;
  digitalWrite(spk[0], LOW);
  digitalWrite(spk[1], LOW);
  interrupts();
}



