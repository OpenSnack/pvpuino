#ifndef PROJECTILE_H
#define PROJECTILE_H

typedef struct {
	float x;
	float y;
	float vertSpeed;
	float horSpeed;
	int size;
	int damage;
	int color;
} Projectile;

#endif