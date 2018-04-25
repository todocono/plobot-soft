
const int dled = 30;
const int trigPin = 19;
const int echoPin = 20;


void setup() {
  pinMode(dled, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(57600);
  
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 10000L);
  distance = (duration/2) / 29.1;
  //if (distance >= 200 || distance <= 0){
  //  Serial.println("Out of range");
//  }
 // else {
    Serial.print(distance);
    Serial.println(" cm");
//  }
  delay(50);
}
