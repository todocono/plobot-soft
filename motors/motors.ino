
#include <WS2812.h>
#define LEDCount 4   // Number of LEDs to drive
WS2812 LED(LEDCount);

//this is for the latest version of 17 MAR 2017 and ahead



const int rst_wifi = 0;   //wifi
const int opt_A = 1;    //opt
const int opt_B = 2;    //opt
const int spk_A = 3;    //spk
const int spk_B = 4;    //spk
const int miso = 5;       //miso

const int motor_l_A = 12;
const int motor_l_B = 13;
const int motor_r_A = 14;
const int motor_r_B = 15;

const int outputPin = 21;    //leds
const int pwr_off   = 29;    //pwr off

int time = 1000;
int speed = 100;

void setup() {

  LED.setOutput(outputPin);
  /* You may uncomment one of the following three lines to switch
    to a different data transmission sequence for your addressable LEDs.
    (These functions can be used at any point in your program as needed.)   */

  LED.setColorOrderRGB();  // Uncomment for RGB color order
  //LED.setColorOrderBRG();  // Uncomment for BRG color order
  //LED.setColorOrderGRB();  // Uncomment for GRB color order (Default; will be used if none other is defined.)

  pinMode(pwr_off, OUTPUT);
  digitalWrite(pwr_off, HIGH);

  pinMode(motor_l_A, OUTPUT);
  pinMode(motor_l_B, OUTPUT);
  pinMode(motor_r_A, OUTPUT);
  pinMode(motor_r_B, OUTPUT);

  /*
    Frequency of oscillation f =  (clock) / (2 * Divisor * TOP)
    These are all for 16MHz, calculate half for 8MHz
    Setting 	Divisor 	Frequency
    0x01 	 	1 	 	31372.55
    0x02 	 	8 	 	3921.16
    0x03          64 	 	490.20   <--DEFAULT
    0x04  	256 	 	122.55
    0x05 	 	1024 	 	30.64
    TCCR1B = (TCCR1B & 0b11111000) | <setting>;

    Setting 	Divisor 	Frequency
    0x01 	 	1  		31372.55
    0x02 	 	8 	 	3921.16
    0x03  	32  		980.39
    0x04 	 	64 	 	490.20   <--DEFAULT
    0x05 	 	128  		245.10
    0x06  	256  		122.55
    0x07 	 	1024  		30.64
    TCCR2B = (TCCR2B & 0b11111000) | <setting>;
  */

  //  TCCR1B = (TCCR1B & 0b11111000) | 0x05;
  //  TCCR2B = (TCCR2B & 0b11111000) | 0x07;

  Serial.begin(57600);
  Serial.println("Gogogo");
}

void loop() {

  set_color(255, 0, 0);
  Serial.println("left front");
  analogWrite(motor_r_A, speed);
  analogWrite(motor_r_B, 0);
  analogWrite(motor_l_A, 0);
  analogWrite(motor_l_B, 0);

  delay(time);

  set_color(0, 0, 0);
  Serial.println("stop");
  digitalWrite(motor_r_A, LOW);
  digitalWrite(motor_r_B, LOW);
  digitalWrite(motor_l_A, LOW);
  digitalWrite(motor_l_B, LOW);
  delay(time);



  set_color(0, 0, 255);
  analogWrite(motor_r_A, 0);
  analogWrite(motor_r_B, speed);
  analogWrite(motor_l_A, 0);
  analogWrite(motor_l_B, 0);
  Serial.println("left back");
  delay(time);

  set_color(0, 0, 0);
  Serial.println("left stop");
  digitalWrite(motor_r_A, LOW);
  digitalWrite(motor_r_B, LOW);
  digitalWrite(motor_l_A, LOW);
  digitalWrite(motor_l_B, LOW);
  delay(time);


  set_color(0, 255, 255);
  analogWrite(motor_r_A, 0);
  analogWrite(motor_r_B, 0);
  analogWrite(motor_l_A, 0);
  analogWrite(motor_l_B, speed);
  Serial.println("right front");
  delay(750);

  set_color(0, 0, 0);
  digitalWrite(motor_r_A, LOW);
  digitalWrite(motor_r_B, LOW);
  digitalWrite(motor_l_A, LOW);
  digitalWrite(motor_l_B, LOW);
  Serial.println("right stop");
  delay(time);

  set_color(255, 255, 0);
  analogWrite(motor_r_A, 0);
  analogWrite(motor_r_B, 0);
  analogWrite(motor_l_A, speed);
  analogWrite(motor_l_B, 0);
  Serial.println("right back");
  delay(time);

  set_color(0, 0, 0);
  digitalWrite(motor_r_A, LOW);
  digitalWrite(motor_r_B, LOW);
  digitalWrite(motor_l_A, LOW);
  digitalWrite(motor_l_B, LOW);
  Serial.println("right stop");
  delay(time);
  Serial.println("off now!");
  //  while (1);
  digitalWrite(pwr_off, LOW);
  delay(time);
}


void set_color(int colorR, int colorG, int colorB) {
  for (int i = 0; i < LEDCount; i++) {
    cRGB value;
    value.b = colorB;
    value.g = colorG;
    value.r = colorR; // RGB Value -> Red Only
    LED.set_crgb_at(i, value); // Set value at LED found at index 0
  }

  LED.sync(); // Sends the data to the LEDs
  delay(10); // Wait (ms)
}
