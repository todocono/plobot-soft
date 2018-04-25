
#include "sense.h"
#include <L3G4200D.h>
#include <Wire.h>

#include <float.h>

const int trigPin = 19;
const int echoPin = 20;
const int micPin = 31;

const int SENSITIVITY = 30; //sensitivity Gyroscope
const int THRESHOLD = 30; //threshold Microphone
const long INTERVAL = 3000; // time to wait for sensing

void init_sense() {
  //for ultrasound
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //for gyro
  Wire.begin();
  // TODO: Hack: First calibration is somehow wrong
  {
    L3G4200D gyroscope;
    while (!gyroscope.begin(L3G4200D_SCALE_250DPS, L3G4200D_DATARATE_400HZ_50))     // Set scale 250 dps and 400HZ Output data rate (cut-off 50)
    {
      Serial.println("Could not find the L3G4200D sensor, check wiring!");
      return;
    }
    gyroscope.calibrate(100);
  }
  //for microphone
  pinMode(micPin, INPUT);
}

float sense_distance_cm() {
  float num = 0, denum = 0;

  for (int s = 0; s < 5; ++s) {
    long duration;
    digitalWrite(trigPin, LOW);  // Added this line
    delayMicroseconds(2); // Added this line
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // Added this line
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH, 10000L);
    float distance = (duration / 2) / 29.1;
    if (distance < 200 && distance > 0) {
      num += distance;
      denum += 1.0f;
    }
    delay(50);
  }

  if (denum == 0.0f)
    return FLT_MAX;

  return num / denum;
}

int sense_mic() {
  int maxSound = 0;
  unsigned long previousMillis = millis();
  
  delay(100);
  while (millis() - previousMillis <= INTERVAL-100) {
    maxSound = max(maxSound, analogRead(micPin));
    delay(1);
  }
  Serial.println(maxSound);
  return maxSound;
}


int sense_gyro() {
  int event = 0;

  L3G4200D gyroscope;
  while (!gyroscope.begin(L3G4200D_SCALE_2000DPS, L3G4200D_DATARATE_400HZ_50))  // Set scale 250 dps and 400HZ Output data rate (cut-off 50)
  {
    Serial.println("Could not find a valid L3G4200D sensor, check wiring!"); 
    return -1;
  }
  gyroscope.calibrate(100);

  gyroscope.setThreshold(3); // Set threshold sensivty. Default 3.

  unsigned long previousMillis = millis();

  while (millis() - previousMillis <= INTERVAL) {
    Vector norm = gyroscope.readNormalize();  // Read normalized values in deg/sec
    if ( abs(norm.XAxis) > SENSITIVITY  ) {
      event = 1;
      return event;
    }
    if ( abs(norm.YAxis) > SENSITIVITY ) {
      event = 2;
      return event;
    }
    if (  abs(norm.ZAxis) > SENSITIVITY ) {
      event = 3;
      return event;
    }
  }
  Serial.println(event);
  return event;
}
