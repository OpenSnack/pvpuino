#ifndef JOYSTICK_H
#define JOYSTICK_H

#define THRESHOLD 2

#include "player.h"

// joystick/button pin defines
// player 1
#define JOYSTICK0_MOVE_HORIZ 0   // Analog input A0 - horizontal
#define JOYSTICK0_MOVE_VERT  1   // Analog input A1 - vertical
#define JOYSTICK0_MOVE_BUTTON 2  // Digital input pin 2 for the button
#define JOYSTICK0_SHOOT_HORIZ 2
#define JOYSTICK0_SHOOT_VERT 3
#define JOYSTICK0_SOUND_BUTTON 3
// player 2
#define JOYSTICK1_MOVE_HORIZ 4
#define JOYSTICK1_MOVE_VERT 5
#define JOYSTICK1_SHOOT_HORIZ 6
#define JOYSTICK1_SHOOT_VERT 7

extern int soundOn;

void getInput(int dt, Player *player);

#endif