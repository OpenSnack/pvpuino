#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "collide.h"
#include "draw.h"
#include "player.h"
#include "powerup.h"
#include "projectile.h"

// generic two-rectangle collision detection, rect1 and rect2
// returns 1 if any two sides are colliding, 0 otherwise
int collide(int x1, int y1, int hor1, int vert1, int x2, int y2, int hor2, int vert2) {
	if(x1 + hor1 > x2 && x1 < x2 + hor2 && y1 + vert1 > y2 && y1 < y2 + vert2) {
		return 1;
	}
	return 0;
}

void checkCollisions(Player *player1, Player *player2) {
	// checks if player1 is hit
	for (int i = 0; i < NUM_PROJECTILES; i++){
		Projectile *proj = &player2->projectiles[i];

		// check for collision only if projectile is on the screen (ie. non-zero speed)
		if(proj->vertSpeed != 0.0 || proj->horSpeed != 0.0) {
			// if player and projectile collide, despawn the projectile
			if(collide(player1->x, player1->y, PLAYER_SIZE, PLAYER_SIZE, proj->x, proj->y, proj->size, proj->size)) {
				player1->health -= proj->damage * player1->defense;
				proj->horSpeed = 0.0; // reset projectile
				proj->vertSpeed = 0.0;
				// redraw the background
				tft.fillRect(proj->x, proj->y, proj->size, proj->size, ST7735_BLACK);
				// redraw player
				tft.fillRect(player1->x, player1->y, PLAYER_SIZE, PLAYER_SIZE, player1->color);
				proj->x = -100.0; // offscreen
				proj->y = -100.0;
				// decrease health bar
				if (player1->ID == 0) {
					tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
					tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, player1->health, HEALTH_BAR_HEIGHT, ST7735_MAGENTA);
				} else if (player1->ID == 1) {
					tft.fillRect(0, 0, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
					tft.fillRect(0, 0, player1->health, HEALTH_BAR_HEIGHT, ST7735_CYAN);
				}
			}
		}
	}

	// check for collision only if there is a spawned powerup
	if (powerUp.onMap == 1) {
		// if player and powerup collide, despawn and apply the powerup
		if(collide(player1->x, player1->y, PLAYER_SIZE, PLAYER_SIZE, powerUp.x, powerUp.y, POWER_UP_SIZE, POWER_UP_SIZE)) {
			tft.fillRect(powerUp.x, powerUp.y, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_BLACK);
			tft.fillRect(player1->x, player1->y, PLAYER_SIZE, PLAYER_SIZE, player1->color);

			// applys power up effect to player
			applyPowerUp(player1, &powerUp);
		}
	}
}