#ifndef POWERUP_H
#define POWERUP_H

#define POWER_UP_SIZE 8

typedef struct {
	uint32_t timer;
	int x;
	int y;
	int type;
	int onMap;
} Power;

extern Power powerUp;

void drawPowerUp(Power* powerUp);
void spawnPowerUp(Power* powerUp);
void applyPowerUp(Player* player, Power* powerUp);
void updatePowerUpState(Player* player);

#endif