/*
 * Plobot was originally designed by Rodolfo Cossovich with the toy design of Camilo Palacios Parra - January 2015
 * Main software structure by Sean Purser-Haskell - 2016
 * Interpreter and conditionals by Rodolfo Cossovich - 2017
 */

#include "bitlash.h"

#include<SPIFlash.h>
#include<SPI.h>

#include <Wire.h>
#include <L3G4200D.h>
#include <PID_v1.h>

#include <WS2812.h>

#include <MFRC522.h>

#include <Servo.h>
#include <SPI.h>

#include "SPIAudio.h"

#include "cards.h"
#include "lights.h"
#include "sound.h"
#include "jingles.h"
#include "sense.h"
#include "move.h"
#include "arms.h"
void set_glow(uint8_t r, uint8_t g, uint8_t b);

IdleGlow idle_glow(2000000L, 255, 255, 255);
const int between_cards_default_pause = 150;  //this is something to review.

const int sStackDepthLimit = 200;

CardSequence main_sequence, stored_sequence;

CardSequence *current_sequence = &main_sequence;

const int battery_level_pin = 26;
const int pwr_off   = 29;    //pwr off
const int battery_low_level = 700;
const float sDeadBatteryVolts = (float(battery_low_level) / 1024.0f) * (3.3f * 3.0f);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillisB = 0;        // will store last time the battery was measured
unsigned long previousMillisT = 0;        // will store last time the battery was measured
#define INTERVAL 3000                     // interval at which to test the battery (milliseconds)
#define TIMEOUT 15 * 20                   // do auto power off ( time = TIMEOUT * INTERVAL )
int timeout = 0;

const int straight_ticks = 800;
const int turn_ticks = 400;

CardId newCard;

void default_glow() {
  set_glow(idle_glow.get_r(), idle_glow.get_g(), idle_glow.get_b());
}

float battery_voltage() {
  const int battery_level = analogRead(battery_level_pin);
  float battery_voltage = float(battery_level) * ((3.3f * 3.0f) / 1024.0f);
  return battery_voltage;
}

void print_battery_level() {
  Serial.print("Battery voltage: ");
  Serial.print(battery_voltage());
  Serial.print("v");
  Serial.println();
}

void do_reset() {
  main_sequence.clear();
  stored_sequence.clear();
  card_scan_jingle(kCardReset);
  idle_glow.glow_rhythm(2);
}

void set_pause_glow(unsigned long paused_start_ms) {
  const float t_since_paused = float(millis() - paused_start_ms) / 1000.0f;
  const float norm_t = t_since_paused / 0.25f;
  const float norm_level = 0.5f * (1.0f + sin(norm_t * 2 * M_PI));
  uint8_t level = uint8_t(128 + 128 * norm_level);
  set_glow(level, level, level);
}

void do_pause_glow(int pause_millis) {
  for (const unsigned long paused_start_ms = millis(); (millis() - paused_start_ms) < pause_millis;) {
    set_pause_glow(paused_start_ms);
  }
}
void setup() {
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  init_movement();
  init_lights();
  init_sound();

  chip_tone(50, 1000, 10);
  chip_tone(25, 800, 10);
  chip_tone(100, 1400, 10);

  initBitlash(57600);                  // Start the interpreter ( and initialize serial communications with the PC )
  Serial.println("com OK");

  SPI.begin();			// Init SPI bus

  //flash.begin();
  SPIAudio::Setup();

  init_cards();
  init_arms();
  init_sense();

  randomSeed(SPIAudio::mic_read());

  pinMode(battery_level_pin, INPUT);
  pinMode(pwr_off, OUTPUT);
  digitalWrite(pwr_off, HIGH);

  set_arms(0.0f, 1.0f, 400);
  { // Shows battery level
    print_battery_level();
    float batt_volts = battery_voltage();
    if (batt_volts > 7.4f) {
      set_glow(0, 255, 0);
    } else if (batt_volts > 7.0f) {
      set_glow(0, 0, 255);
    } else if (batt_volts > sDeadBatteryVolts) {
      set_glow(255, 255, 0);
      delay (200);
      set_glow(255, 255, 0);
      delay (200);
      set_glow(255, 255, 0);
    }
    delay(200);
  }
  addBitlashFunction("servo", (bitlash_function) servoHandler);
  addBitlashFunction("card", (bitlash_function) cardHandler);
  addBitlashFunction("ultra", (bitlash_function) ultraSound);
  addBitlashFunction("sound", (bitlash_function) chipTone);
  addBitlashFunction("gyro", (bitlash_function) gyroSense);
  addBitlashFunction("listen", (bitlash_function) micSense);
  addBitlashFunction("rec", (bitlash_function) recPlay);

  Serial.println("starting...");
  newCard = kCardNull;
  previousMillisT = millis();

}

void execute_sequence(CardSequence const&sequence, int depth = 1) {
  static float arm_pos = 0.65f;
  boolean paused = false;
  unsigned long paused_start_ms = 0;

  // If condition is -1, the action is not taken.
  int condition = 0;

  default_glow();

  for (int card_idx = 0; card_idx < sequence.count();) {
    Serial.print("Condition IS ");
    Serial.println(condition);
    unsigned long pause_millis = 1;

    const CardId card = sequence.atIndex(card_idx);
    if (!paused) {
      if (condition != -1) {
        switch (card) {
          case kCardForward:
            move_straight(straight_ticks);
            break;
          case kCardBackward:
            move_straight(-straight_ticks);
            break;
          case kCardLeft:
            turn(90);
            pause_millis = 200;
            break;
          case kCardRight:
            turn(-90);
            pause_millis = 200;
            break;
          case kCardLift:
            set_arms(arm_pos, 1.0f - arm_pos, 400);
            arm_pos = (arm_pos == 0.0f) ? 0.65f : 0.0f;
            pause_millis = between_cards_default_pause;
            break;
          case kCardPause:
            do_pause_glow(1000);
            break;
          case kCardSound:
            if ( sequence.atIndex(card_idx - 1) == kCardListen ) { //previous card was Listen
              Serial.println("Recording..");
              Serial.flush();
              recSPI();
            }
            else {
              Serial.println("Playing..");
              Serial.flush();
              playSPI();
            }
            break;
          case kCardRandom:
            condition = (int) random(1, 10);
            break;
          case kCardSee:
            condition = (sense_distance_cm() > 10.0f) ? -1 : 1;
            break;
          case kCardListen:
            /* Note for this kind of notation
              Find max(a, b) is the same as
              max = ( a > b ) ? a : b;  */
            condition = (sense_mic() < 400) ? -1 : 1;
            break;
          case kCardFeel:
            condition = (sense_gyro() < 1) ? -1 : 1;
            break;
          case kCardRepeat:
            // TODO: Infinite recursion
            if (&sequence != &stored_sequence ) { //&& condition == 0) {  //last minute modification to allow to repeat 
              if (depth <= sStackDepthLimit) {
                execute_sequence(stored_sequence, depth + 1);
              } else {
                // Refuse to recurse too deeply
                error_jingle();
              }
            } else {
              card_idx = -1;    // NOTE: THIS MUST BE UNCONDITIONAL
              condition = 0;    // Support infinite recursion without stack overflow
            }
            break;
          default: {
              if (is_note_card(card)) {
                play_note(card);
                pause_millis = 100;
              }
              break;
            }
        }
      }
      if (!card_sets_condition(card)) {
        condition = 0;
      }
      ++card_idx;
      default_glow();
    } else if (paused) {
      set_pause_glow(paused_start_ms);
      pause_millis = 1;
    }

    for (const unsigned long smillis = millis(); (millis() - smillis) < pause_millis;) {
      CardId scanned = read_one_card();
      if (scanned == kCardReset) {
        do_reset();
        return;
      } else if (scanned == kCardStart) {
        card_idx = 0;  // Loop will increment
        paused = false;
        card_scan_jingle(kCardStart);
        break;
      } else if (scanned == kCardPause) {
        paused = !paused;
        pause_jingle(paused);
        paused_start_ms = millis();
      }
    }
  }
}

void do_go() {
  if (!main_sequence.empty()) {
    card_scan_jingle(kCardStart);
  } else {
    error_jingle();
    return;
  }
  execute_sequence(main_sequence);
finished_actions:
  flush_cards();
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisB >= INTERVAL) {
    previousMillisB = currentMillis;
    float batt_volts = battery_voltage();       //low battery protection
    if (batt_volts > 7.4f) {
      set_glow(0, 255, 0);
    } else if (batt_volts > 7.0f) {
      set_glow(0, 0, 255);
    } else if (batt_volts > sDeadBatteryVolts) {
      set_glow(255, 0, 0);
      delay(2000);
      Serial.println("dead battery!");
      digitalWrite(pwr_off, LOW);
    }

    if ( timeout++ > TIMEOUT ) {
      Serial.println("switching off...");
      digitalWrite(pwr_off, LOW);
      delay(1);
    }
  }


  //run interpreter
  runBitlash();

  // Glow
  idle_glow.do_glow();

  // Card reading
  CardId scanned;
  if ( newCard ) {
    Serial.println(newCard);
    scanned = newCard;
    newCard =   kCardNull;
  }
  else {
    scanned = read_one_card();
  }

  if (scanned) {    //did we scan a new card? or did it entered through the serial port?
    timeout = 0;   // while new cards are scanned, don't go to sleep
    Serial.println(scanned);
    // Card handling _glow
    if (is_key_card(scanned)) {
      if (scanned == kCardRed) {
        idle_glow.set(255, 0, 0);
        Serial.println("kCardRed");
      } else if (scanned == kCardGreen) {
        idle_glow.set(0, 255, 0);
        Serial.println("kCardGreen");
      } else if (scanned == kCardBlue) {
        idle_glow.set(0, 0, 255);
        Serial.println("kCardBlue");
      } else if (scanned == kCardOrange) {
        idle_glow.set(255, 128, 0);
        Serial.println("kCardOrange");
      } else if (scanned == kCardPink) {
        idle_glow.set(255, 0, 255);
        Serial.println("kCardPink");
      } else if (scanned == kCardYellow) {
        idle_glow.set(255, 200, 0);
        Serial.println("kCardYellow");
      } else if (scanned == kCardPurple) {
        idle_glow.set(127, 0, 255);
        Serial.println("kCardPurple");

        // Card handling _execution
      } else if (scanned == kCardStart) {
        do_go();
        Serial.println("kCardStart");
      } else if (scanned == kCardReset) {
        do_reset();
        Serial.println("kCardReset");
      } else if (scanned == kCardStore) {
        idle_glow.glow_rhythm(7);         
        if (current_sequence == &main_sequence) {
          stored_sequence.clear();
          current_sequence = &stored_sequence;
          subroutine_jingle(false);
        } else if (current_sequence == &stored_sequence) {
          idle_glow.glow_rhythm(2);
          current_sequence = &main_sequence;
          subroutine_jingle(true);
        }
      } else if (scanned == kCardBoost) {
        //   do_boost();
        Serial.println("kCardBoost");
      }
    } else {
      if (!current_sequence->queue(scanned)) {
        error_jingle();
      } else {
        card_scan_jingle(scanned);
      }
    }
  }
}

//this is a prototype that demonstrates adding an interpreter using Bitlash ( http://bitlash.net )

#define NUMSERVOS 8
byte servo_install_count;	// number of servos that have been initialized
byte servo_pin[NUMSERVOS];	// the pins of those servos
Servo myservos[NUMSERVOS];	// the Servo object for thos

//////////
// servohandler: function handler for Bitlash servo() function
//
//	arg 1: servopin
//	arg 2: setting
//
numvar servoHandler(void) {
  byte slot = 0;
  // is this pin already allocated a servo slot?
  while (slot < servo_install_count) {
    if (servo_pin[slot] == getarg(1)) break;
    slot++;
  }
  if (slot >= servo_install_count) {	// not found

    // do we have a free slot to allocate?
    if (servo_install_count < NUMSERVOS) {
      slot = servo_install_count++;
      servo_pin[slot] = getarg(1);
      myservos[slot].attach(getarg(1));
    }
    else {
      Serial.println("servohandler: oops!");	// please increase NUMSERVOS above
      return -1;
    }
  }
  myservos[slot].write(getarg(2));
  return 0;
}



//////////
// cardHandler: function handler for Bitlash card() function
//
//	arg 1: vector with cards
//      arg 1: needed?
//
numvar cardHandler(void) {
  Serial.print("cards to upload: ");
  Serial.print(getarg(1));
  newCard = raw_int_card_id(getarg(1)); // newCard = getarg(1);
  //  Serial.print("card read: ");
  //  Serial.println(newCard);
  /* if (getarg(1) >= 10) {	//  found?

    }
    else {
     Serial.println("error !");
     return -1;
    }
  */
  //do something else
  // myservos[slot].write(getarg(2));
  return 0;
}

//////////
// ultraSound: function handler for Bitlash ultra() function
/////////
numvar ultraSound(void) {
  init_sense();
  Serial.print("distance ahead: ");
  Serial.println(sense_distance_cm());
  //do something else
  // myservos[slot].write(getarg(2));

  return 0;
}

///////////
//// tone: function handler for Bitlash sound( duration, tone, volume) function
///////////
numvar chipTone(void) {
  init_sound();
  chip_tone(getarg(1), getarg(2), getarg(3));
  return 0;
}


///////////
//// micSense: function handler for Bitlash listen() function
///////////
numvar micSense(void) {
  init_sound();
  Serial.println(sense_mic());
  return 0;
}

///////////
//// gyroSense: function handler for Bitlash gyro() function
///////////
numvar gyroSense(void) {
  // init_sense();
  Serial.print("gyro answer: ");
  Serial.println((int)sense_gyro());
  //do something else
  // myservos[slot].write(getarg(2));
  return 0;
}

///////////
//// recPlay: function handler for Bitlash rec() function
///////////
numvar recPlay(void) {
  chip_tone(300, 1000, 30);
  Serial.println("Recording..");
  Serial.flush();
  recSPI();

  chip_tone(500, 1000, 30);
  Serial.println("Playing..");
  Serial.flush();
  playSPI();
  return 0;
}
