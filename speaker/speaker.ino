
const int pin_spk[2] = {3, 4};
const int mtr_pins[4] = {12, 13, 14, 15};

void chip_tone(unsigned long for_millis, unsigned long period_micros, int volume) {
  const uint16_t period_cycles = ((((unsigned long)F_CPU) / 1000000L) / 4) * period_micros;
  const uint16_t half_delay_cycles = period_cycles / 2;
  const uint16_t play_iters = (for_millis * 1000L) / period_micros;
  
  // Off: don't burn speaker
  OCR0A = 0;
  OCR0B = 0;

  noInterrupts();
  const auto ra_save = TCCR0A;
  const auto rb_save = TCCR0B;
  TCCR0A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR0B = _BV(CS22);
  
  TCCR0B = (TCCR0B & 0b11111000) | 0x01;

  for(uint16_t i=0;i<play_iters;++i) {
    OCR0B = 0;
    OCR0A = volume;
    _delay_loop_2(half_delay_cycles);

    OCR0A = 0;
    OCR0B = volume;
    _delay_loop_2(half_delay_cycles);
  }

  OCR0A = 0;
  OCR0B = 0;

  TCCR0A = ra_save;
  TCCR0B = rb_save;
  interrupts();
}

void go_jingle() {
  chip_tone(300, 1250, 10);
  delay(200);
  chip_tone(300, 1250, 255);
  delay(200);
  chip_tone(300, 800, 100);
}

void setup() {
  Serial.begin(57600);
  pinMode(pin_spk[0], OUTPUT);
  pinMode(pin_spk[1], OUTPUT);

  pinMode(mtr_pins[0], OUTPUT);
  pinMode(mtr_pins[1], OUTPUT);
  pinMode(mtr_pins[2], OUTPUT);
  pinMode(mtr_pins[3], OUTPUT);
}

void loop() {

  while(true) {
    Serial.println("Playing...");
    go_jingle();
    Serial.println("Waiting...");
    delay(500);
  }
}
