
//#include <Adafruit_NeoPixel.h>
#include <WS2812.h>
#include "jingles.h"

#include "lights.h"
#include "sound.h"
#include "cards.h"

void scan_success_jingle()
{
  // Indicate success
  for(int i=0;i<4;++i) {
    chip_tone(100, 2500 - 500 * i);
    set_glow(0,25 + 75 * i,0);
  }
}

void error_jingle() {
  set_glow(255, 0, 0);
  chip_tone(500, 5000);
}

void reset_jingle() {
  const float tmax = 2.75f;
  for(float t=0;t<tmax;t+=0.1f) {
    const float n = sin(t * 2 * M_PI);
    const float scale = 1.0f - (t / tmax);
    chip_tone(100, 750 + 750 * (0.5f + 0.5f * n), scale * DEFAULT_VOLUME);
    set_glow(max(0, n*scale*255),0,max(0, -n*scale*255));
  }
}

void go_jingle() {
  set_glow(64,64,0);
  chip_tone(300, 1250);
  set_glow(0,0,0);
  delay(200);
  set_glow(255,255,0);
  chip_tone(300, 1250);
  set_glow(0,0,0);
  delay(200);
  set_glow(0,255,0);
  chip_tone(300, 800);
  set_glow(0,0,0);
}

void pause_jingle(boolean pausing) {
  set_glow(0,0,0);
  if(pausing) {
    set_glow(255,255,255);
    chip_tone(150, 800);
    delay(150);
    set_glow(200,200,200);
    chip_tone(250, 1000);
    delay(200);
    set_glow(128,128,128);
    chip_tone(400, 2000);
  } else {
    set_glow(128,128,128);
    chip_tone(400, 2000);
    delay(200);
    set_glow(200,200,200);
    chip_tone(250, 1000);
    delay(150);
    set_glow(255,255,255);
    chip_tone(150, 800);
    delay(150);
  }
}

const int sNotePeriods[] = {
  3830,
  3400,
  3038,
  2864,
  2550,
  2272,
  2028,
};

const uint32_t sNoteColors[] = {
/*  get_color(255,64,64),
  get_color(255,128,0),
  get_color(255,255,0),
  get_color(0,255,0),
  get_color(0,204,204),
  get_color(127,0,255),
  get_color(255,0,127),
  */
};

void play_note(CardId action_id) {
  int filtered_action_id = max(kCardMusicDo, min(kCardMusicSi, (int)action_id));
  const unsigned long period = sNotePeriods[filtered_action_id - kCardMusicDo];
//  set_glow(sNoteColors[filtered_action_id - (int)kCardMusicDo]);
  chip_tone(300, period);
}

void card_scan_jingle(CardId card) {
  switch(card) {
    case kCardStart:
      go_jingle();
      break;
    case kCardReset:
      reset_jingle();
      break;
    case kCardMusicDo:
    case kCardMusicRe:
    case kCardMusicMi:
    case kCardMusicFa:
    case kCardMusicSol:
    case kCardMusicLa:
    case kCardMusicSi:
      play_note(card);
      break;
    case kCardOrange:
    case kCardRed:
    case kCardGreen:
    case kCardPurple:
    case kCardBlue:
      // No jingle
      break;
    default:
      scan_success_jingle();
      break;
  }
}

void subroutine_jingle(boolean at_end)
{
  if(at_end) {
    set_glow(100,100,0);
    chip_tone(300, 1000);
    set_glow(70,70,0);
    chip_tone(300, 1300);
  } else {
    set_glow(70,70,0);
    chip_tone(300, 1300);
    set_glow(100,100,0);
    chip_tone(300, 1000);
  }
}




