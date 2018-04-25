// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED
// UNTESTED





#include <WS2812.h>

#define outputPin 21 // Digital output pin (default: 7)
#define LEDCount 4  // Number of LEDs to drive (default: 9)
const int mic = 31;

WS2812 LED(LEDCount);
cRGB value;

byte intensity;
byte sign;

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(57600);
  LED.setOutput(outputPin); // Digital Pin 7

  /* You may uncomment one of the following three lines to switch
  to a different data transmission sequence for your addressable LEDs.
  (These functions can be used at any point in your program as needed.)   */

  LED.setColorOrderRGB();  // Uncomment for RGB color order
  //LED.setColorOrderBRG();  // Uncomment for BRG color order
  //LED.setColorOrderGRB();  // Uncomment for GRB color order (Default; will be used if none other is defined.)

  intensity = 0;
  sign = 1;
}

void loop() {


  sensorValue = analogRead(mic);
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 0, 340, 255, 0);
  // change the analog out value:
  // analogWrite(analogOutPin, outputValue);

  // print the results to the serial monitor:
  Serial.print("sensor = " );
  Serial.print(sensorValue);
  Serial.print("\t output = ");
  Serial.println(outputValue);

  intensity = outputValue;

  int i = 0;
  if (sign) {
    //intensity++;
    if (intensity == 255)
      sign = 0;
  }
  else {
    intensity--;
    if (intensity == 0)
      sign = 1;
  }

  while (i < LEDCount) {
    if ((i % 3) == 0) {  // First LED, and every third after that
      value.b = 0;
      value.g = 0;
      value.r = intensity; // RGB Value -> Red Only
      LED.set_crgb_at(i, value); // Set value at LED found at index 0
    }
    else if ((i % 3) == 1) { // Second LED, and every third after that
      value.b = 0;
      value.g = intensity;
      value.r = 0; // RGB Value -> Green Only
      LED.set_crgb_at(i, value); // Set value at LED found at index 0
    }
    else  { // Third LED, and every third after that
      value.b = intensity;
      value.g = 0;
      value.r = 0; // RGB Value -> Blue Only
      LED.set_crgb_at(i, value); // Set value at LED found at index 0
    }
    i++;
  }

  LED.sync(); // Sends the data to the LEDs
  delay(10); // Wait (ms)
}
