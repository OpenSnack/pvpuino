#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "collide.h"
#include "draw.h"
#include "player.h"
#include "powerup.h"
#include "sound.h"

Power powerUp;

void drawPowerUp(Power* powerUp) {
	switch(powerUp->type) {
		case 0: 
			tft.drawRect(powerUp->x, powerUp->y, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_RED);
			tft.fillRect(powerUp->x, powerUp->y + 3, POWER_UP_SIZE, 2, ST7735_RED);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, POWER_UP_SIZE, ST7735_RED);
			break;
		case 1:
			tft.drawRect(powerUp->x, powerUp->y, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_BLUE);
			tft.fillRect(powerUp->x, powerUp->y + 3, POWER_UP_SIZE, 2, ST7735_BLUE);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, POWER_UP_SIZE, ST7735_BLUE);
			break;
		case 2:
			tft.drawRect(powerUp->x, powerUp->y, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_YELLOW);
			tft.fillRect(powerUp->x, powerUp->y + 3, POWER_UP_SIZE, 2, ST7735_YELLOW);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, POWER_UP_SIZE, ST7735_YELLOW);
			break;
		case 3:
			tft.drawRect(powerUp->x, powerUp->y, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_GREEN);
			tft.fillRect(powerUp->x, powerUp->y + 3, POWER_UP_SIZE, 2, ST7735_GREEN);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, POWER_UP_SIZE, ST7735_GREEN);
			break;
	}
}

void spawnPowerUp(Power* powerUp) {
	if(millis() - powerUp->timer > 10000 && powerUp->onMap == 0) {
		if (random(2)){
			
			int randomX = random(127-POWER_UP_SIZE);
			int randomY = random(127-POWER_UP_SIZE);

			Wall wall;
			int safeWalls = 0;

			for(int i = 0; i < numWalls[currentLevel]; i++) {
				wall = currentWalls[i];
				if (collide(randomX, randomY, POWER_UP_SIZE, POWER_UP_SIZE, wall.x, wall.y, wall.width, wall.height)){
					break;
				}
				safeWalls++;
			}

			if (safeWalls == numWalls[currentLevel]){
				powerUp->x = min(randomX, SCREEN_WIDTH - POWER_UP_SIZE);
				powerUp->y = constrain(randomY, HEALTH_BAR_HEIGHT*2, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2 - POWER_UP_SIZE);
				powerUp->type = random(4);
				powerUp->onMap = 1;	
				playSound(5);
			}
		}
	}

	// redraws power up if it remains active to prevent bullets from
	// drawing over it
	if(powerUp->onMap){
		drawPowerUp(powerUp);
	}
}

void applyPowerUp(Player* player, Power* powerUp) {
	playSound(6);
	int newHealth = player->health + 30;
	switch (powerUp->type) {
		case 0:
			player->health = min(128, newHealth);
			// updates health bar
			if (player->ID == 0) {
				tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
				tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, player->health, HEALTH_BAR_HEIGHT, ST7735_MAGENTA);
			} else if (player->ID == 1){
				tft.fillRect(0, 0, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
				tft.fillRect(0, 0, player->health, HEALTH_BAR_HEIGHT, ST7735_CYAN);
			}
			break;
		case 1:
			player->defense = 0;
			player->color = tft.Color565(0x33, 0x99, 0xFF);
			break;
		case 2:
			player->damageModifier = 2;
			player->color = tft.Color565(0xFF, 0xFF, 0x33);
			break;
		case 3:
			player->burstLimit = 0;
			player->color = tft.Color565(0x33, 0xFF, 0x33);
			break;
	}

	if (powerUp->type > 0) {
		if (player->ID == 0) {
			tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_YELLOW);
		} else if (player->ID == 1){
			tft.fillRect(0, HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_YELLOW);
		}
		player->powerUpTimer = millis();
	}

	// allows another power up to spawn
	powerUp->onMap = 0;
	powerUp->timer = millis();
}

void updatePowerUpState(Player* player){
	int powerUpWidth;
	int powerUpDuration = millis() - player->powerUpTimer;

	if (player->powerUpTimer != -1 && powerUpDuration > 5000){
		// returns player to normal state
		player->defense = 1;
		player->damageModifier = 1;
		player->burstLimit = 100;
		player->powerUpTimer = -1;
		if(player->ID == 0) {
			player->color = ST7735_MAGENTA;
		} else if(player->ID == 1) {
			player->color = ST7735_CYAN;
		}
		if (player->ID == 0) {
			tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, wallColors[currentLevel]);
		} else if (player->ID == 1) {
			tft.fillRect(0, HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, wallColors[currentLevel]);
		}
	} else if (player->powerUpTimer != -1){
		powerUpWidth = map(powerUpDuration, 0, 5000, 0, SCREEN_WIDTH);
		
		if (player->ID == 0) {
			tft.fillRect(SCREEN_WIDTH - powerUpWidth, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, powerUpWidth, HEALTH_BAR_HEIGHT, wallColors[currentLevel]);
		} else if (player->ID == 1) {
			tft.fillRect(SCREEN_WIDTH - powerUpWidth, HEALTH_BAR_HEIGHT, powerUpWidth, HEALTH_BAR_HEIGHT, wallColors[currentLevel]);
		}

	}
}