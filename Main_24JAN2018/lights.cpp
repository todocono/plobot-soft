
#include "lights.h"

//#include <Adafruit_NeoPixel.h>

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, 21, NEO_GRB + NEO_KHZ800);

//we replaced adafruit library because it was not working in 8MHz
// new library is documented at https://github.com/cpldcpu/light_ws2812
#include <WS2812.h>

#define outputPin 21 // Digital output pin (default: 7)
#define LEDCount 4   // Number of LEDs to drive (default: 9)

WS2812 LED(LEDCount);
cRGB value;
/*
  cRGB get_color(byte r, byte g, byte b) {
  //return strip.Color(r,g,b);
  return cRGB
  }
*/

byte rhythm = 2;

void init_lights() {
  //  strip.begin();
  //  set_glow(0);
  LED.setOutput(outputPin);

  LED.setColorOrderRGB();  // Uncomment for RGB color order

  rhythm = 2;
}

void set_glow(uint8_t r, uint8_t g, uint8_t b) {
  //  const uint32_t c = strip.Color(r, g, b);
  value.b = b;
  value.g = r;
  value.r = g; // RGB Value -> Red Only
  set_glow(value);
}

void set_glow(cRGB c) {
  //  strip.setPixelColor(0, c);
  //  strip.setPixelColor(1, c);
  //  strip.show();
  LED.set_crgb_at(0, value); // Set value at LED found at index 0
  LED.set_crgb_at(1, value); // Set value at LED found at index 1
  LED.set_crgb_at(2, value); // Set value at LED found at index 2
  LED.set_crgb_at(3, value); // Set value at LED found at index 3
  LED.set_crgb_at(4, value); // Set value at LED found at index 4
  LED.sync(); // Sends the data to the LEDs
}

IdleGlow::IdleGlow(unsigned long glow_period_micros,
                   byte glow_r,
                   byte glow_g,
                   byte glow_b)
  : glow_period_micros(glow_period_micros),
    last_glow_micros(micros()),
    glow_t_micros(0L),
    glow_r(glow_r),
    glow_g(glow_g),
    glow_b(glow_b) {
}

void IdleGlow::do_glow() {
  const unsigned long m = micros();
  const unsigned long glow_t_micros_offset = m - last_glow_micros;
  glow_t_micros += glow_t_micros_offset;
  last_glow_micros = m;
  float glow_t = float(glow_t_micros) / glow_period_micros;

  float glow_n = 0.5f - 0.5f * cos(glow_t * rhythm * M_PI);

  set_glow(glow_n * glow_r, glow_n * glow_g, glow_n * glow_b);
}

void IdleGlow::set(byte glow_r,
                   byte glow_g,
                   byte glow_b) {
  this->glow_r = glow_r;
  this->glow_g = glow_g;
  this->glow_b = glow_b;
}

float IdleGlow::get_r()const {
  return glow_r;
}

float IdleGlow::get_g()const {
  return glow_g;
}


float IdleGlow::get_b()const {
  return glow_b;
}

void IdleGlow::glow_rhythm(byte new_rhythm) {
  rhythm = new_rhythm;
}


