
#ifndef ARMS_H
#define ARMS_H

void init_arms();

// pos is -50 to 50
// duration is in millis
void set_arms(float left_pos_norm, float right_pos_norm, int duration);

#endif//ARMS_H

