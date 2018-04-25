#ifndef SENSE_H
#define SENSE_H

#include <Arduino.h>

void init_sense();

// Returns FLT_MAX when nothing is seen
float sense_distance_cm();

// Returns the mic value
int sense_mic();

// Returns FLT_MAX when there's no big XYZ change
int sense_gyro();

#endif//SENSE_H

