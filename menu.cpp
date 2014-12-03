#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "draw.h"
#include "joystick.h"
#include "menu.h"
#include "player.h"
#include "powerup.h"
#include "sound.h"

int gameState = 0;

void initializeGame() {
	// intializes player properties
	for (int i = 0; i < NUM_PLAYERS; i++){
		players[i].health = 128;
		players[i].defense = 1;
		players[i].damageModifier = 1;
		players[i].burstLimit = 100;
		// -1 state represents not having a powerUp
		players[i].powerUpTimer = -1;
		players[i].shootTimer = millis();

		// resets projectiles
		for (int k = 0; k < NUM_PROJECTILES; k++) {
			players[i].projectiles[k].vertSpeed = 0;
			players[i].projectiles[k].horSpeed = 0;
			players[i].projectiles[k].y = -100.0;
			players[i].projectiles[k].x = -100.0;
		}
	}

	players[0].color = ST7735_MAGENTA;
	players[1].color = ST7735_CYAN;

	tft.fillScreen(ST7735_BLACK);

	// load chosen level
	switch (currentLevel) {
		case 0:
			currentWalls = level0;
			players[0].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[0].y = 3*SCREEN_HEIGHT/4;
			players[1].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[1].y = SCREEN_HEIGHT/4 - PLAYER_SIZE;
			break;
		case 1:
			currentWalls = level1;
			players[0].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[0].y = SCREEN_HEIGHT - 15 - PLAYER_SIZE;
			players[1].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[1].y = 15;
			break;
		case 2:
			currentWalls = level2;
			players[0].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[0].y = 3*SCREEN_HEIGHT/4;
			players[1].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[1].y = SCREEN_HEIGHT/4 - PLAYER_SIZE;
			break;
		case 3:
			currentWalls = level3;
			players[0].x = SCREEN_WIDTH - 20 - PLAYER_SIZE;
			players[0].y = SCREEN_HEIGHT - 40 - PLAYER_SIZE;
			players[1].x = 20;
			players[1].y = 40;
			break;
		case 4:
			currentWalls = level4;
			players[0].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[0].y = SCREEN_HEIGHT - 40 - PLAYER_SIZE;
			players[1].x = SCREEN_WIDTH/2 - PLAYER_SIZE/2;
			players[1].y = 40;
			break;
	}

	drawWalls(currentWalls, currentLevel);

	// draw initial players
	tft.fillRect(players[0].x, players[0].y, PLAYER_SIZE, PLAYER_SIZE, players[0].color);
	tft.fillRect(players[1].x, players[1].y, PLAYER_SIZE, PLAYER_SIZE, players[1].color);

	// height boundaries
	tft.fillRect(0, 0, SCREEN_WIDTH, HEALTH_BAR_HEIGHT*2, wallColors[currentLevel]);
	tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, SCREEN_WIDTH, HEALTH_BAR_HEIGHT*2, wallColors[currentLevel]);

	// health bars
	tft.fillRect(0, 0, players[1].health, HEALTH_BAR_HEIGHT, ST7735_CYAN);
	tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, players[0].health, HEALTH_BAR_HEIGHT, ST7735_MAGENTA);

	// draws countdown
	// 3
	playSound(3);
	tft.fillRect(54, 68, 20, 20, ST7735_WHITE);
	tft.fillRect(54, 72, 16, 4, ST7735_BLACK);
	tft.fillRect(54, 80, 16, 4, ST7735_BLACK);
	delay(1000);

	// 2
	playSound(3);
	tft.fillRect(54, 68, 20, 20, ST7735_WHITE);
	tft.fillRect(54, 72, 16, 4, ST7735_BLACK);
	tft.fillRect(58, 80, 16, 4, ST7735_BLACK);
	delay(1000);
	// 1 
	playSound(3);
	tft.fillRect(54, 68, 20, 20, ST7735_BLACK);
	tft.fillRect(62, 68, 4, 20, ST7735_WHITE);
	delay(1000);

	//go
	playSound(4);
	tft.fillRect(62, 68, 4, 20, ST7735_BLACK);
	tft.fillRect(42, 64, 20, 20, ST7735_WHITE);
	tft.fillRect(46, 68, 16, 4, ST7735_BLACK);
	tft.fillRect(46, 72, 4, 8, ST7735_BLACK);
	tft.fillRect(50, 76, 8, 4, ST7735_BLACK);

	tft.fillRect(66, 64, 20, 20, ST7735_WHITE);
	tft.fillRect(70, 68, 12, 12, ST7735_BLACK);
	delay(300);

	tft.fillRect(38, 64, 66, 20,  ST7735_BLACK);
	// finishes drawing countdown

	drawWalls(currentWalls, currentLevel);

	// enters gameplay state
	powerUp.timer = millis();
	powerUp.onMap = 0;
	gameState = 2;
}

void mapSelection() {
	int mapID = 0;

	tft.fillScreen(ST7735_BLACK);
	tft.setCursor((SCREEN_WIDTH - 84)/2 + 10, 12);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Select Map");

    tft.fillRect((SCREEN_WIDTH - 84)/2, 32, 84, 16, wallColors[0]);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, ST7735_WHITE);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, ST7735_WHITE);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, ST7735_WHITE);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 128, 84, 16, ST7735_WHITE);

	tft.setTextWrap(false);

	tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 36);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Simple");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 60);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Arena");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 84);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Boxes");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 108);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Plus++");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 12, 132);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Final Dest.");

	// infinte loop until an option is selected
 	uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;
		getInput(dt, &players[0]);

		if (players[0].vertMove < -THRESHOLD && mapID == 1) {
			playSound(1);
			mapID = 0;

			tft.fillRect((SCREEN_WIDTH - 84)/2, 32, 84, 16, wallColors[0]);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 36);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Simple");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Arena");

		    delay(200);

		} else if ((players[0].vertMove > THRESHOLD && mapID == 0) || (players[0].vertMove < -THRESHOLD && mapID == 2)) {
			playSound(1);
			mapID = 1;

			tft.fillRect((SCREEN_WIDTH - 84)/2, 32, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 32, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, wallColors[1]);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 36);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Simple");

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Arena");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Boxes");

		    delay(200);

		} else if ((players[0].vertMove > THRESHOLD && mapID == 1) || (players[0].vertMove < -THRESHOLD && mapID == 3)) {
			playSound(1);
			mapID = 2;

			tft.fillRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 56, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, wallColors[2]);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Arena");

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Boxes");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Plus++");

		    delay(200);

		} else if ((players[0].vertMove > THRESHOLD && mapID == 2) || (players[0].vertMove < -THRESHOLD && mapID == 4)) {
			playSound(1);
			mapID = 3;

			tft.fillRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 80, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, wallColors[3]);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 128, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 128, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Boxes");

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Plus++");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 12, 132);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Final Dest.");

		    delay(200);

		} else if (players[0].vertMove > THRESHOLD && mapID == 3) {
			playSound(1);
			mapID = 4;

			tft.fillRect((SCREEN_WIDTH - 84)/2, 128, 84, 16, wallColors[4]);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 104, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Plus++");

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 12, 132);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Final Dest.");

		    delay(200);

		}

 		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
 			playSound(2);
			break;
		}

 		startTime = currentTime;
 	} 

 	currentLevel = mapID;
 	initializeGame();
}

void mainMenu() {
	int menuSelection = 1;

	// draws main menu screen
	tft.fillScreen(ST7735_BLACK);
	// P
	tft.fillRect(12, 32, 4, 24, ST7735_MAGENTA);
	tft.fillRect(16, 32, 4, 4, ST7735_MAGENTA);
	tft.fillRect(20, 36, 4, 4, ST7735_MAGENTA);
	tft.fillRect(16, 40, 4, 4, ST7735_MAGENTA);
	// v
	tft.fillRect(28, 44, 4, 4, ST7735_WHITE);
	tft.fillRect(30, 48, 4, 4, ST7735_WHITE);
	tft.fillRect(32, 52, 4, 4, ST7735_WHITE);
	tft.fillRect(34, 48, 4, 4, ST7735_WHITE);
	tft.fillRect(36, 44, 4, 4, ST7735_WHITE);
	// P
	tft.fillRect(48, 32, 4, 24, ST7735_CYAN);
	tft.fillRect(52, 32, 4, 4, ST7735_CYAN);
	tft.fillRect(56, 36, 4, 4, ST7735_CYAN);
	tft.fillRect(52, 40, 4, 4, ST7735_CYAN);
	// u
	tft.fillRect(64, 44, 12, 12, ST7735_WHITE);
	tft.fillRect(68, 44, 4, 8, ST7735_BLACK);
	// i
	tft.fillRect(80, 36, 4, 4, ST7735_WHITE);
	tft.fillRect(80, 44, 4, 12, ST7735_WHITE);
	// n
	tft.fillRect(88, 42, 4, 2, ST7735_WHITE);
	tft.fillRect(88, 44, 12, 12, ST7735_WHITE);
	tft.fillRect(92, 48, 4, 8, ST7735_BLACK);
	// o
	tft.fillRect(104, 44, 12, 12, ST7735_WHITE);
	tft.fillRect(108, 48, 4, 4, ST7735_BLACK);

	tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);

	tft.setTextWrap(false);

	tft.setCursor((SCREEN_WIDTH - 84)/2 + 28, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("PLAY!");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Instructions");
    playSound(0);
    //end of drawing

    // infintie loop until an option is selected
    uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt, &players[0]);
		// update postion on menu, updates program state
		if (players[0].vertMove > THRESHOLD && menuSelection != 0) {
			playSound(1);
			//highlight second button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 28, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("PLAY!");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Instructions");

			menuSelection = 0;
		} else if (players[0].vertMove < -THRESHOLD && menuSelection != 1){
			playSound(1);
			//highlight first button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 28, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("PLAY!");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Instructions");
			
			menuSelection = 1;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			playSound(2);
			break;
		}

		startTime = currentTime;
	}

	if (menuSelection == 0){
		gameState = 1;
	} else {
		mapSelection();
	}

}

void instructionsMenu() {
	int page = 0;
 	int initialLoad = 1;

 	// infintie loop until an option is selected
 	uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;
		getInput(dt, &players[0]);

		if (initialLoad == 1 || players[0].vertMove < -THRESHOLD && page == 1) {
				initialLoad = 0;
				page = 0;

				tft.fillScreen(ST7735_BLACK);
				tft.setTextWrap(true);
				tft.setCursor(0,0);
    			tft.setTextColor(ST7735_YELLOW);
				tft.print("How to Play PVPuino!\n\n");
				tft.setTextColor(ST7735_WHITE);
				tft.print("Objective:\n");
				tft.print("Aim shots at the other player until their health is depleted.\n\n");
				tft.print("Basic Controls:\n");
				tft.print("Left Joystick: Moves Player\n");
				tft.print("Right Joystick: Aims Shot\n\n");
				tft.print("Menu Controls:\n");
				tft.print("Player 1's move joystick is used to navigate the menu. ");
				tft.print("Clicking the joystick will select the highlighted\n");

				tft.setCursor(0, 152);
    			tft.setTextColor(ST7735_GREEN);
    			tft.print("Page 1/2");

		} else if (players[0].vertMove > THRESHOLD && page == 0) {
				page = 1;

				tft.fillScreen(ST7735_BLACK);
				tft.setTextWrap(true);
				tft.setCursor(0,0);
    			tft.setTextColor(ST7735_WHITE);
				
				tft.print("menu button. Sound can be disabled by clicking Player 1's shoot joystick.\n\n");
    			tft.print("Power Ups:\n\n");

    			tft.drawRect(10, 53, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_RED);
				tft.fillRect(10, 56, POWER_UP_SIZE, 2, ST7735_RED);
				tft.fillRect(13, 53, 2, POWER_UP_SIZE, ST7735_RED);

				tft.drawRect(26, 53, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_BLUE);
				tft.fillRect(26, 56, POWER_UP_SIZE, 2, ST7735_BLUE);
				tft.fillRect(29, 53, 2, POWER_UP_SIZE, ST7735_BLUE);

				tft.drawRect(42, 53, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_YELLOW);
				tft.fillRect(42, 56, POWER_UP_SIZE, 2, ST7735_YELLOW);
				tft.fillRect(45, 53, 2, POWER_UP_SIZE, ST7735_YELLOW);

				tft.drawRect(58, 53, POWER_UP_SIZE, POWER_UP_SIZE, ST7735_GREEN);
				tft.fillRect(58, 56, POWER_UP_SIZE, 2, ST7735_GREEN);
				tft.fillRect(61, 53, 2, POWER_UP_SIZE, ST7735_GREEN);

				tft.setCursor(0,68);
    			tft.setTextColor(ST7735_RED);
				tft.print("Red: Restores health\n");

				tft.setTextColor(ST7735_BLUE);
				tft.print("Blue: Grants invincibility\n");

				tft.setTextColor(ST7735_YELLOW);
				tft.print("Yellow: Double damage");

				tft.setTextColor(ST7735_GREEN);
				tft.print("Green: Bullet burst mode\n");

				tft.setCursor(0, 152);
    			tft.setTextColor(ST7735_GREEN);
    			tft.print("Page 2/2");		
    	}

 		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			playSound(2);
			break;
		}

 		startTime = currentTime;
 	} 

 	gameState = 0;  
}

void pauseMenu(){
	gameState = 2;
	playSound(2);
	int enterTime = millis();

	// draws pause menu
	tft.setTextWrap(false);

	tft.setCursor((SCREEN_WIDTH - 84)/2 + 10, 50);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Game Paused");

	tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_BLACK);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);
	tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);

	tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Resume");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Exit to Menu");
    // end of drawing

    // stop multiple presses
    delay(200);

	uint32_t startTime = millis();
	// infinite loop until an option is selected
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt, &players[0]);
		// update postion on menu, updates program state
		if (players[0].vertMove > THRESHOLD && gameState != 0) {
			playSound(1);
			//highlight second button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Resume");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			gameState = 0;
		} else if (players[0].vertMove < -THRESHOLD && gameState!= 2){
			playSound(1);
			//highlight first button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 24, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("Resume");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Exit to Menu");
			
			gameState = 2;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			playSound(2);
			break;
		}

	}

	// draws the gameplay screen back on
	if (gameState == 2) {
		tft.fillScreen(ST7735_BLACK);
		tft.fillRect(players[0].x, players[0].y, PLAYER_SIZE, PLAYER_SIZE, players[0].color);
		tft.fillRect(players[1].x, players[1].y, PLAYER_SIZE, PLAYER_SIZE, players[1].color);

		// height boundaries
		tft.fillRect(0, 0, SCREEN_WIDTH, HEALTH_BAR_HEIGHT*2, wallColors[currentLevel]);
		tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, SCREEN_WIDTH, HEALTH_BAR_HEIGHT*2, wallColors[currentLevel]);

		// health bars
		tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
		tft.fillRect(0, 0, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_BLACK);
		tft.fillRect(0, 0, players[1].health, HEALTH_BAR_HEIGHT, ST7735_CYAN);
		tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT, players[0].health, HEALTH_BAR_HEIGHT, ST7735_MAGENTA);

		// walls
		drawWalls(currentWalls, currentLevel);

		// updates power up timers
		int timeDiff = millis() - enterTime;

		for (int i = 0; i < NUM_PLAYERS; i++){
			if (players[i].powerUpTimer != -1) {
				players[i].powerUpTimer += timeDiff;
				if (players[i].ID == 0) {
					tft.fillRect(0, SCREEN_HEIGHT - HEALTH_BAR_HEIGHT*2, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_YELLOW);
				} else if (players[i].ID == 1){
					tft.fillRect(0, HEALTH_BAR_HEIGHT, SCREEN_WIDTH, HEALTH_BAR_HEIGHT, ST7735_YELLOW);
		}
			}
		}
		powerUp.timer += timeDiff;
	}
}

void endMenu(int playerID) {
	int menuSelection = 1;

	// draws pause menu
	tft.setTextWrap(false);

	tft.setCursor(38, 40);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Game Over\n");
    tft.setCursor(23, 60);
    tft.print("Player ");
    tft.print(playerID + 1);
    tft.print(" Wins!");

    playSound(7);

	tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_BLACK);
	tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);
	tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);

	tft.setCursor((SCREEN_WIDTH - 84)/2 + 18, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("New Game");

    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Exit to Menu");

    // win counter
    tft.fillRect(0, SCREEN_HEIGHT - 16, SCREEN_WIDTH, 16, ST7735_WHITE);
    tft.setCursor(5, SCREEN_HEIGHT - 12);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Wins: ");
    tft.setTextColor(ST7735_MAGENTA);
    tft.print("P1:");
    tft.print(players[0].wins);
    tft.setTextColor(ST7735_BLUE);
    tft.print("  P2:");
    tft.print(players[1].wins);

    // end of drawing

	uint32_t startTime = millis();
	// infinite loop until an option is selected
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt, &players[0]);
		// update postion on menu, updates program state
		if (players[0].vertMove > THRESHOLD && menuSelection != 0) {
			playSound(1);
			//highlight second button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 18, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("New Game");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			menuSelection = 0;
		} else if (players[0].vertMove < -THRESHOLD && menuSelection != 1){
			playSound(1);
			//highlight first button
			tft.fillRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_BLACK);
			tft.drawRect((SCREEN_WIDTH - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((SCREEN_WIDTH - 84)/2, 78, 84, 16, ST7735_WHITE);

			tft.setCursor((SCREEN_WIDTH - 84)/2 + 18, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("New Game");

		    tft.setCursor((SCREEN_WIDTH - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Exit to Menu");
			
			menuSelection = 1;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			playSound(2);
			break;
		}
	}
	if (menuSelection == 0){
		gameState = 0;
	} else {
		mapSelection();
	}
}