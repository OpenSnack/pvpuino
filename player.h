#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#include "projectile.h"

#define PLAYER_SIZE 8
#define NUM_PLAYERS 2
#define NUM_PROJECTILES 20

typedef struct {
	int ID;
	float x;
	float y;
	uint32_t color;
	int health;
	int defense;
	int damageModifier;
	int burstLimit;
	uint32_t powerUpTimer;
	uint32_t shootTimer;
	int horMove;
	int vertMove;
	float horSpeed;
	float vertSpeed;
	int horShoot;
	int vertShoot;
	int naturalVertMove;
	int naturalHorMove;
	int naturalVertShoot;
	int naturalHorShoot;
	int wins;
	Projectile projectiles[NUM_PROJECTILES];
} Player;

extern Player players[NUM_PLAYERS];

#endif