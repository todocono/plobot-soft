
#ifndef LIGHTS_H
#define LIGHTS_H

#include <Arduino.h>
#include <WS2812.h>

void init_lights();
void set_glow(byte r, byte g, byte b);
void set_glow(cRGB c);
//uint32_t get_color(byte r, byte g, byte b);

class IdleGlow {
public:
  IdleGlow(unsigned long glow_period_micros, 
           byte glow_r, 
           byte glow_g, 
           byte glow_b);
  void set(byte glow_r, 
           byte glow_g, 
           byte glow_b);
  float get_r()const;
  float get_g()const;
  float get_b()const;
  void do_glow();
  void glow_rhythm(byte new_rhythm);
private:
  const unsigned long glow_period_micros;
  unsigned long last_glow_micros;
  unsigned long glow_t_micros;
  
  float glow_r, glow_g, glow_b;
};

#endif//LIGHTS_H

