
#ifndef SPI_AUDIO_H
#define SPI_AUDIO_H

#include <SPIFlash.h>

namespace SPIAudio {

void Setup();

enum AudioRate {
  AudioRate_Null=0,
  AudioRate_16khz,
  AudioRate_32khz,
};

// This is a blocking function
// Uses Timer0!
boolean StreamBlocks(SPIFlash &flash, unsigned long fromBlock, unsigned long count, AudioRate rate);

// This is a blocking function
boolean RecordBlocks(SPIFlash &flash, unsigned long fromBlock, unsigned long count, AudioRate rate);

int mic_read();

void Start();
void End();

}
void recSPI();
void playSPI();

#endif//SPI_AUDIO_H

