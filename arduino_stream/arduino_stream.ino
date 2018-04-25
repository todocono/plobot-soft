
#include<SPIFlash.h>
#include<SPI.h>

#define DEBUG_PRINT 1

const int motor_l_en = 13;
const int motor_r_en = 12;

const int motor_l_dir = 24;
const int motor_r_dir = 25;

const int pin_spk_en = 15;
const int pin_spk_dir = 23;

#define cs 18
#define RFID_NSS_PIN          14         // Configurable, see typical pin layout above

int strPage, strSize;
byte strOffset;

SPIFlash flash(cs);

unsigned int pageIndex = 0;
char read_buffer[256];
unsigned int read_buf_off = 0;

void ResetPage() {
  memset(&read_buffer[0], 0, sizeof(read_buffer));
  read_buf_off = 0;
}

void setup() {
  pinMode(pin_spk_dir, OUTPUT);
  pinMode(pin_spk_en, OUTPUT);

  pinMode(motor_l_en, OUTPUT);
  pinMode(motor_r_en, OUTPUT);
  pinMode(motor_l_dir, OUTPUT);
  pinMode(motor_r_dir, OUTPUT);
  
  pinMode(RFID_NSS_PIN, OUTPUT);
  digitalWrite(RFID_NSS_PIN, HIGH);
  pinMode(cs, OUTPUT);
  digitalWrite(cs, LOW);

  Serial.begin(57600);

  flash.begin();
  ResetPage();
  
  Serial.println("---- Streaming Upload ready ----");
}

void loop() {
  if(Serial.available()) {
    int to_read = min(256 - read_buf_off, Serial.available());
    Serial.readBytes(&read_buffer[read_buf_off], to_read);
    read_buf_off += to_read;
    if(read_buf_off == 256) {
      if(pageIndex % 256 == 0) {
        if(!flash.eraseBlock64K(1+pageIndex, 0)) {
#if DEBUG_PRINT
          Serial.print("Error on eraseBlock ");
          Serial.print(1+pageIndex);
          Serial.println();
#endif
        }
      }
      if(!flash.writePage(1+pageIndex, (uint8_t*)read_buffer)) {    
#if DEBUG_PRINT
        Serial.print("2 Page write error (");
        Serial.print(1+pageIndex);
        Serial.println(") ");
#endif
      }
      ResetPage();
      ++pageIndex;

    }
  }
}

