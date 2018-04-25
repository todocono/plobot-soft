#include <Wire.h>
#include <L3G4200D.h>
#include <PID_v1.h>

#include "move.h"

const int pin_spk_en = 12;
const int pin_spk_dir = 25;




const int motor_l_pulse = 28;
const int motor_r_pulse = 27;

const int motor_l_en = 13;
//const int motor_r_en = 12;
const int motor_r_en = 15;

const int motor_l_dir = 24;
//const int motor_r_dir = 25;
const int motor_r_dir = 23;



void setup() {
  pinMode(pin_spk_dir, OUTPUT);
  pinMode(pin_spk_en, OUTPUT);

  Serial.begin(57600);
  init_movement();

  delay(1000);
}

void loop() {

  int   min_power = 180;

  digitalWrite(motor_l_dir, LOW);
  digitalWrite(motor_r_dir, LOW);

  analogWrite(motor_r_en, min_power);
  analogWrite(motor_l_en, min_power);

delay(1000);


  digitalWrite(motor_l_dir, HIGH);
  digitalWrite(motor_r_dir, HIGH);

delay(1000);


// move_straight(1300);



  /*
  delay(500);
  Serial.println("Turning R...");
  turn(90);

  Serial.println("Turning L...");
  turn(-90);
  delay(500);


  Serial.println("Moving sraight...");
  move_straight(1300);
  delay(500);
  Serial.println("Moving sraight...");
  move_straight(-1300);
  delay(500);
  */
}
