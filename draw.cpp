#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "draw.h"
#include "collide.h"
#include "joystick.h"
#include "powerup.h"
#include "sound.h"

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

Wall level0[4] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 200},{128, -20, 30, 200}};
Wall level1[8] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 200},{128, -20, 30, 200},{36, 34, 56, 4},{36, 122, 56, 4},{20, 52, 4, 56},{104, 52, 4, 56}};
Wall level2[10] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 200},{128, -20, 30, 200},{28, 32, 22, 22},{18, 76, 28, 28},{36, 124, 10, 10},{88, 24, 14, 14},{70, 54, 22, 22},{76, 96, 34, 34}};
Wall level3[14] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 200},{128, -20, 30, 200},{62, 22, 4, 12},{62, 48, 4, 12},{62, 74, 4, 12},{62, 100, 4, 12},{62, 126, 4, 12},{10, 78, 12, 4},{34, 78, 12, 4},{58, 78, 12, 4},{82, 78, 12, 4},{106, 78, 12, 4}};
Wall level4[5] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 200},{128, -20, 30, 200},{14, 74, 100, 14}};
int numWalls[5] = {4, 8, 10, 14, 5};
int wallColors[5];

Wall *currentWalls;
int currentLevel;

void drawWalls(Wall *level, int currentLevel) {
	for(int i = 4; i < numWalls[currentLevel]; i++) {
		tft.fillRect(level[i].x, level[i].y, level[i].width, level[i].height, wallColors[currentLevel]);
	}
}

// return 1 if we just shot
int newProjectile(int dt, Player *player, int size, int damage) {
	for(int i = 0; i < NUM_PROJECTILES; i++) {
		Projectile *proj = &player->projectiles[i];
		float vertShoot = (float)player->vertShoot; /* trust me, we need these two lines */
		float horShoot = (float)player->horShoot; /* don't ask why */
		if(proj->vertSpeed == 0.0 && proj->horSpeed == 0.0) {
			proj->size = size;
			proj->damage = damage;
			proj->color = player->color;
			// this normalizes the shooting input so that magnitude of projectile speed doesn't change
			proj->x = player->x + (float)PLAYER_SIZE/2.0 - (float)proj->size/2.0;
			proj->y = player->y + (float)PLAYER_SIZE/2.0 - (float)proj->size/2.0;
			float square = (float)abs((vertShoot * vertShoot) + (horShoot * horShoot));
			proj->vertSpeed = vertShoot / (float)sqrt(square) / 8.0;
			proj->horSpeed = horShoot / (float)sqrt(square) / 8.0;
			tft.fillRect(proj->x, proj->y, proj->size, proj->size, player->color);
			return 1;
		}
	}
	return 0;
}

void moveProjectile(int dt, Projectile *proj) {

	tft.fillRect((int)proj->x, (int)proj->y, proj->size, proj->size, ST7735_BLACK);
	
	proj->x += proj->horSpeed * dt;
	proj->y += proj->vertSpeed * dt;
	
	Wall wall;
	for(int i = 0; i < numWalls[currentLevel]; i++) {
		wall = currentWalls[i];
		if(collide(proj->x, proj->y, proj->size, proj->size, wall.x, wall.y, wall.width, wall.height)) {
			proj->vertSpeed = 0.0;
			proj->horSpeed = 0.0;
			proj->x = -100.0; // offscreen
			proj->y = -100.0;
			return;
		}
	}
	tft.fillRect((int)proj->x, (int)proj->y, proj->size, proj->size, proj->color);
}

void updateCharacters(int dt, Player *player) {
	float newX = player->x;
	float newY = player->y;
	
	Wall wall;
	int touching = 0;

	// down movement
	if(player->vertMove > THRESHOLD) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newY + PLAYER_SIZE > wall.y - player->vertSpeed && newY + PLAYER_SIZE < wall.y + wall.height - player->vertSpeed) {
				if(newX + PLAYER_SIZE > wall.x && newX < wall.x + wall.width) {
					newY = (float)(wall.y - PLAYER_SIZE);
					touching = 1;
					break;
				}
			}
		}
		if(!touching) {
			newY += player->vertSpeed;
		}
		touching = 0;
	}

	// up movement
	if(player->vertMove < -THRESHOLD) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newY < wall.y + wall.height - player->vertSpeed && newY > wall.y - player->vertSpeed) {
				if(newX + PLAYER_SIZE > wall.x && newX < wall.x + wall.width) {
					newY = (float)(wall.y + wall.height);
					touching = 1;
					break;
				}
			}
		}
		if(!touching) {
			newY += player->vertSpeed;
		}
		touching = 0;
	}

	// right movement
	if(player->horMove > THRESHOLD) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newX + PLAYER_SIZE > wall.x - player->horSpeed && newX + PLAYER_SIZE < wall.x + wall.width - player->horSpeed) {
				if(newY + PLAYER_SIZE > wall.y && newY < wall.y + wall.height) {
					newX = (float)(wall.x - PLAYER_SIZE);
					touching = 1;
					break;
				}
			}
		}
		if(!touching) {
			newX += player->horSpeed;
		}
		touching = 0;
	}

	// left movement
	if(player->horMove < -THRESHOLD) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newX < wall.x + wall.width - player->horSpeed && newX > wall.x - player->horSpeed) {
				if(newY + PLAYER_SIZE > wall.y && newY < wall.y + wall.height) {
					newX = (float)(wall.x + wall.width);
					touching = 1;
					break;
				}
			}
		}
		if(!touching) {
			newX += player->horSpeed;
		}
		touching = 0;
	}

	// draw the player
	// prevents blinking by only redrawing the character if we moved
	if (abs(player->vertMove) > THRESHOLD || abs(player->horMove) > THRESHOLD) {
		tft.fillRect(player->x, player->y, PLAYER_SIZE, PLAYER_SIZE, ST7735_BLACK);
		tft.fillRect(newX, newY, PLAYER_SIZE, PLAYER_SIZE, player->color);
	}

	player->x = newX;
	player->y = newY;

	updatePowerUpState(player);
}

void updateProjectiles(int dt, Player *player) {
	int shooting;
	if(abs(player->vertShoot) > THRESHOLD || abs(player->horShoot) > THRESHOLD) {
		if(millis() - player->shootTimer > player->burstLimit) {
			shooting = newProjectile(dt, player, 2, DEFAULT_DAMAGE*player->damageModifier);
			player->shootTimer = millis();
		}
	}

	for(int i = 0; i < NUM_PROJECTILES; i++) {
		moveProjectile(dt, &player->projectiles[i]);
	}
	if(shooting) {
		tft.fillRect(player->x, player->y, PLAYER_SIZE, PLAYER_SIZE, player->color);
	}
}