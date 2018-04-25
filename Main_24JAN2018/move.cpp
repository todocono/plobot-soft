
#include "move.h"
#include <Wire.h>
#include <L3G4200D.h>
#include <PID_v1.h>

const int motor_l_pulse = 27;
const int motor_r_pulse = 28;

const int motor_l_a = 12; 
const int motor_l_b = 13;

const int motor_r_a = 15;   //this was inverted in the latest version of October PCB
const int motor_r_b = 14;

const int MAX_POWER = 140;
const int MIN_POWER = 70;

class PulseCounter
{ public:
    PulseCounter(int pin)
      : pin_(pin),
        pulses_(0), last_state_(digitalRead(pin))
    {
    }

    void count() {
      const int state = digitalRead(pin_);

      if (state == LOW && last_state_ == HIGH) {
        // Pulse start
        last_state_ = LOW;
      } else if (state == HIGH && last_state_ == LOW) {
        // Pulse end
        last_state_ = HIGH;

        ++pulses_;
      }
    }

    int pulses()const {
      return pulses_;
    }
  private:
    int pin_;

    int last_state_;
    int pulses_;
};

PulseCounter count_left(motor_l_pulse);
PulseCounter count_right(motor_r_pulse);

void pciSetup(byte pin)
{
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR (PCINT0_vect)
{
  noInterrupts();
  count_left.count();
  count_right.count();
  interrupts();
}



void init_movement() {

  pinMode(motor_l_pulse, INPUT);
  pinMode(motor_r_pulse, INPUT);

  pinMode(motor_l_a, OUTPUT);
  pinMode(motor_l_b, OUTPUT);
  pinMode(motor_r_a, OUTPUT);
  pinMode(motor_r_b, OUTPUT);

  pciSetup(motor_l_pulse);
  pciSetup(motor_r_pulse);

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
}

void turn(int degs)
{
  //we clear all counters related to the timers we will adjust
  TCNT1H = 0; // set timer1 high byte to (0 - (8 - 1))
  TCNT1L = 0;
  TCNT2 = 0; // set timer2 to 0
  GTCCR = GTCCR | 0x01; //reset prescaler

  /*
    Frequency of oscillation f =  (clock) / (2 * Divisor * TOP)
    These are all for 16MHz, calculate half for 8MHz
    Setting   Divisor   Frequency
    0x01    1     31372.55
    0x02    8     3921.16
    0x03    64    490.20   <--DEFAULT
    0x04    256     122.55
    0x05    1024    30.64
    TCCR1B = (TCCR1B & 0b11111000) | <setting>;

    Setting   Divisor   Frequency
    0x01    1     31372.55
    0x02    8     3921.16
    0x03    32      980.39
    0x04    64    490.20   <--DEFAULT
    0x05    128     245.10
    0x06    256     122.55
    0x07    1024      30.64
    TCCR2B = (TCCR2B & 0b11111000) | <setting>;
  */

  TCCR1B = (TCCR1B & 0b00111000) | 0x01;  //see table above
  TCCR2B = (TCCR2B & 0b00111000) | 0x01;

  unsigned long amount_off_target ;
  L3G4200D gyroscope;
  while (!gyroscope.begin(L3G4200D_SCALE_250DPS, L3G4200D_DATARATE_400HZ_50))  // Set scale 250 dps and 400HZ Output data rate (cut-off 50)
  {
    Serial.println("Could not find a valid L3G4200D sensor, check wiring!");
    return;
  }
  gyroscope.calibrate(100);

  count_left = PulseCounter(motor_l_pulse);
  count_right = PulseCounter(motor_r_pulse);

  double Setpoint = 0, Input = 0, Output = 0;
  const double scale_factor = 7000000.0f;
  //original values PID myPID(&Input, &Output, &Setpoint, 9.5 / scale_factor, 4 / scale_factor, 3.5 / scale_factor, DIRECT);
  PID myPID(&Input, &Output, &Setpoint, 11 / scale_factor, 5.5 / scale_factor, 4 / scale_factor, DIRECT); //Kp Ki  Kd
  //PID myPID(&Input, &Output, &Setpoint, 0.01/ scale_factor, 0 / scale_factor, 0 / scale_factor, DIRECT); //Kp Ki  Kd
  myPID.SetMode(AUTOMATIC);
  // Max should be <255 for bootstrap
  myPID.SetOutputLimits(-65, 65);
  int min_pwr = MIN_POWER + 25;
  //  int max_power = min_pwr + 25;

  long z_total = 0;
  Input = 0;
  const long target_z = long(float(degs) * (90000000.0f / 90.0f));
  unsigned long last_off_target = millis();
  unsigned long last_read_sensor = micros();
  const unsigned long started_turn = millis();

  Setpoint = target_z;
  // Grace period to start moving
  long last_not_moving = millis() + 50;

  int direction;

  while ((millis() - last_off_target) < 100L && (millis() - started_turn) < 2000L)
  {
    const int max_pulses = max(count_left.pulses(), count_right.pulses());
    const long now_millis = millis();
    const long bump_min_speed_frequency = 10;
    if (max_pulses < 5 && (now_millis - last_not_moving) > bump_min_speed_frequency) {
      last_not_moving += bump_min_speed_frequency;
      min_pwr += 5;
    }

    amount_off_target = abs(z_total - target_z);
    if (amount_off_target > 2506516L) {
      last_off_target = millis();
    }

    const unsigned long read_sensor_time = micros();
    Vector norm = gyroscope.readNormalize();
    const long time_since_read = (read_sensor_time - last_read_sensor);
    last_read_sensor = read_sensor_time;

    long yaw = (long)norm.ZAxis;
    z_total += yaw * time_since_read;

    Input = double(z_total);
    myPID.Compute();

    if (Output > 0) {
      direction = 1;
    }
    else {
      direction = -1;
    }

    int mtr_pwr = min_pwr + abs(Output);

    //        Serial.print("target: ");
    //        Serial.print(target_z);
    //        Serial.print("  input: ");
    //        Serial.print(Input);
    //        Serial.print("  out ");
    //        Serial.println(Output);

    const int iclp = count_left.pulses();
    const int icrp = count_right.pulses();
    const int encoder_diff = int(iclp) - int(icrp);

    if (abs(encoder_diff) > 5) {
      if (iclp > icrp) {
        motorRight( mtr_pwr, -direction);
        motorLeft( min_pwr, direction);
      }
      else {
        motorRight( min_pwr, -direction);
        motorLeft( mtr_pwr, direction);
      }
    }
    else {
      motorRight( mtr_pwr, -direction);
      motorLeft( mtr_pwr, direction);
    }
  }
  motorRight( 0, direction);
  motorLeft( 0, direction);
  //
  //  Serial.print("input: ");
  //  Serial.println(Input);
  //  Serial.print("out ");
  //  Serial.println(Output);
  //  Serial.print("min power ");
  //  Serial.println(min_pwr);
  //  Serial.print("amount ");
  //  Serial.println(amount_off_target);
  //
  //  Serial.print("pulses left ");
  //  Serial.println(count_left.pulses());
  //  Serial.print("pulses right ");
  //  Serial.println(count_right.pulses());
  ////
  //  //return the timers to their regular configuration
  //  TCCR1B = (TCCR1B & 0b00000000) | 0x03;
  //  TCCR2B = (TCCR2B & 0b00000000) | 0x04;

  TCNT1H = 0; // set timer1 high byte to (0 - (8 - 1))
  TCNT1L = 0;
  TCNT2 = 0; // set timer2 to 0
  GTCCR = GTCCR | 0x01; //reset prescaler

  TCCR1B = (TCCR1B & 0b00111000) | 0x03;
  TCCR2B = (TCCR1B & 0b00111000) | 0x04;
}

void move_straight(int pulses)
{
  TCNT1H = 0; // set timer1 high byte to (0 - (8 - 1))
  TCNT1L = 0;
  TCNT2 = 0; // set timer2 to 0
  GTCCR = GTCCR | 0x01; //reset prescaler

  TCCR1B = (TCCR1B & 0b00111000) | 0x03;
  TCCR2B = (TCCR1B & 0b00111000) | 0x04;


  count_left = PulseCounter(motor_l_pulse);    //reset the counters of pulses from encoders
  count_right = PulseCounter(motor_r_pulse);

  //  Serial.print("initial left pulses ");
  //  Serial.println(count_left.pulses());
  //  Serial.print("initial right pulses ");
  //  Serial.println(count_right.pulses());

  const unsigned long sm = micros();
  int direction;
  if (pulses >= 0) {
    direction = 0;
  } else {
    direction = 1;
  }

  pulses = abs(pulses);

  unsigned long last_off_target = millis();
  const unsigned long started_move = millis();

  // Max should be <255 for bootstrap
  int min_pwr = MIN_POWER, max_pwr = MAX_POWER;
  // Grace period to start moving
  long last_not_moving = millis() + 100;

  while ((millis() - last_off_target) < 500 && (millis() - started_move) < 2000L) {
    const long now_millis = millis();
    const int max_pulses = max(count_left.pulses(), count_right.pulses());
    const int iclp = count_left.pulses();  //from last time we reset the counters, how many pulses?
    const int icrp = count_right.pulses();
    const boolean mismatch = abs(int(iclp) - int(icrp)) > 5;
    const boolean arrived = max_pulses >= pulses;

    if (!arrived || mismatch) {
      last_off_target = millis();
    }

    const int avg_pulses = (count_left.pulses() + count_right.pulses()) / 2;
    //const int triangle = (avg_pulses > pulses / 2) ? 2 * (pulses - avg_pulses) : 2 * avg_pulses; // decreasing speed(breaking) : increasing(accelerating)   (used to be for the original S+I motor model (pulses - 16 * avg_pulses) : 2*avg_pulses; )
    const int triangle = (avg_pulses > pulses / 2) ? (pulses - 2 * avg_pulses) : avg_pulses;
    const int mtr_speed = max(min_pwr, min(max_pwr, min_pwr + triangle));

    const long bump_min_speed_frequency = 20;
    if (max_pulses < 5 && (now_millis - last_not_moving) > bump_min_speed_frequency) {
      last_not_moving += bump_min_speed_frequency;
      min_pwr += 5;
      max_pwr += 5;
    }

   // Serial.println( mtr_speed);

    if (!arrived) {
      if (mismatch) {
        if (iclp >  icrp) {
          motorLeft( 0, direction);
          motorRight( mtr_speed , direction);
        } else {
          motorLeft( mtr_speed, direction);
          motorRight( 0, direction);
        }
      } else {
        motorRight( mtr_speed, direction);
        motorLeft( mtr_speed, direction);
      }
    } else {
      if (abs(int(iclp) - int(icrp)) < 15) {
        motorRight( 0, direction);
        motorLeft( 0, direction);
      } else {
        if (iclp > icrp) {
          motorLeft(  0, direction);
          motorRight( min_pwr , direction);
        } else {
          motorLeft( min_pwr, direction);
          motorRight( 0, direction);
        }
      }
    }
  }
  motorRight( 0, direction);
  motorLeft( 0, direction);
  //for debug only
  //
  //  Serial.print("left pulses ");
  //  Serial.println(count_left.pulses());
  //  Serial.print("right pulses ");
  //  Serial.println(count_right.pulses());
  //  Serial.print("Min power at ");
  //  Serial.println(min_pwr);
  //  Serial.print("Max power at ");
  //  Serial.println(max_pwr);

}


void motorLeft( int speed, int direction) {
  if (speed > 235) speed = 235;
  if (speed < 20) {
    analogWrite(motor_l_a, 0);
    analogWrite(motor_l_b, 0);
  } else {
    if (direction < 1) {
      analogWrite(motor_l_a, 0);
      analogWrite(motor_l_b, speed);
    } else {
      analogWrite(motor_l_a, speed);
      analogWrite(motor_l_b, 0);
    }
  }
}

void motorRight( int speed, int direction) {
  if (speed > 235) speed = 235;
  if (speed < 20) {
    analogWrite(motor_r_a, 0);
    analogWrite(motor_r_b, 0);
  } else {
    if (direction < 1) {
      analogWrite(motor_r_a, 0);
      analogWrite(motor_r_b, speed);
    } else {
      analogWrite(motor_r_a, speed);
      analogWrite(motor_r_b, 0);
    }
  }
}
