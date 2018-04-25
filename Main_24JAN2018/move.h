
#ifndef MOVE_H
#define MOVE_H

void init_movement();
// Sign determines CW / CCW
void turn(int degs);
// Sign determines forward/back
void move_straight(int pulses);

void motorRight( int speed, int direction);
void motorLeft(  int speed, int direction);

#endif//MOVE_H

