
#include<SPIFlash.h>
#include<SPI.h>

// Before running this test, run:
//  ln -s ../../SPIAudio/SPIAudio.cpp
//  ln -s ../../SPIAudio/SPIAudio.h
#include "SPIAudio.h"

const int dled = 30;

const int motor_l_en = 13;
const int motor_r_en = 12;

const int motor_l_dir = 24;
const int motor_r_dir = 25;

const int pin_spk_en = 15;
const int pin_spk_dir = 23;

#define cs 18
#define RFID_NSS_PIN          14         // Configurable, see typical pin layout above

SPIFlash flash(cs);

void setup() {
  Serial.begin(57600);
  
  pinMode(pin_spk_dir, OUTPUT);
  pinMode(pin_spk_en, OUTPUT);

  pinMode(motor_l_en, OUTPUT);
  pinMode(motor_r_en, OUTPUT);
  pinMode(motor_l_dir, OUTPUT);
  pinMode(motor_r_dir, OUTPUT);  
  
  pinMode(dled, OUTPUT);
  
  flash.begin();
  
  SPIAudio::Setup();
}

void loop() {
  const uint16_t start_block = 40*256;
  const uint16_t n_blocks = 250;
  
  Serial.println("Erasing 32k..");
  Serial.flush();
  for(int i=0;i<(1+250/128);++i) {
    if(!flash.eraseBlock32K(start_block+i*128,0)) {
      Serial.println("Error erasing");
      return;
    }
  }
  
  Serial.println("Recording..");
  Serial.flush();
  if(!SPIAudio::RecordBlocks(flash, start_block, n_blocks, SPIAudio::AudioRate_16khz)) {
    Serial.println("Error recording");
    return;
  }
  
  Serial.println("Playing..");
  Serial.flush();
  if(!SPIAudio::StreamBlocks(flash, start_block, n_blocks, SPIAudio::AudioRate_16khz)) {
    Serial.println("Error streaming blocks");
    return;
  }
}
