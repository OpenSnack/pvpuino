#include <Arduino.h>
#include "joystick.h"
#include "player.h"

int soundOn = 1;

// receives all input from the joysticks associated with the player
void getInput(int dt, Player *player) {
	// finds the difference between the current joystick position and the
	// calibrated center
	if(player->ID == 0) {
		player->vertMove = analogRead(JOYSTICK0_MOVE_VERT) - player->naturalVertMove;
		player->horMove = analogRead(JOYSTICK0_MOVE_HORIZ) - player->naturalHorMove;
		player->vertSpeed = player->vertMove * dt / 1000.0 / 8.0;
		player->horSpeed = player->horMove * dt / 1000.0 / 8.0;

		player->vertShoot = analogRead(JOYSTICK0_SHOOT_VERT) - player->naturalVertShoot;
		player->horShoot = analogRead(JOYSTICK0_SHOOT_HORIZ) - player->naturalHorShoot;
	} else {
		player->vertMove = -(analogRead(JOYSTICK1_MOVE_VERT) - player->naturalVertMove);
		player->horMove = -(analogRead(JOYSTICK1_MOVE_HORIZ) - player->naturalHorMove);
		player->vertSpeed = player->vertMove * dt / 1000 / 8.0;
		player->horSpeed = player->horMove * dt / 1000 / 8.0;

		player->vertShoot = -(analogRead(JOYSTICK1_SHOOT_VERT) - player->naturalVertShoot);
		player->horShoot = -(analogRead(JOYSTICK1_SHOOT_HORIZ) - player->naturalHorShoot);
	}

	// controls mute/unmute of sound
	if(!digitalRead(JOYSTICK0_SOUND_BUTTON)) {
		soundOn = !soundOn;
		delay(200);
	}
}