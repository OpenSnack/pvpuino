// Steven Boddez & George Antonious, CMPUT 274 Section EA1

#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "collide.h"
#include "draw.h"
#include "joystick.h"
#include "menu.h"
#include "player.h"
#include "powerup.h"

void setup() {
	Serial.begin(9600);
	randomSeed(analogRead(15));

	// initialize wallColors
	wallColors[0] = tft.Color565(0xFF, 0xFF, 0xFF);
	wallColors[1] = tft.Color565(0xFF, 0x00, 0x00);
	wallColors[2] = tft.Color565(0x00, 0xFF, 0x00);
	wallColors[3] = tft.Color565(0xFF, 0x8D, 0x00);
	wallColors[4] = tft.Color565(0x00, 0x00, 0xFF);

	// initialize display and joysticks
	tft.initR(INITR_BLACKTAB); 
	digitalWrite(JOYSTICK0_MOVE_BUTTON, HIGH);
	digitalWrite(JOYSTICK0_SOUND_BUTTON, HIGH);

	for(int i = 0; i < NUM_PLAYERS; i++) {
		players[i].ID = i;
	}

	// calibrates the center postion of the joysticks
	players[0].naturalVertMove = analogRead(JOYSTICK0_MOVE_VERT);
	players[0].naturalHorMove = analogRead(JOYSTICK0_MOVE_HORIZ);
	players[0].naturalVertShoot = analogRead(JOYSTICK0_SHOOT_VERT);
	players[0].naturalHorShoot = analogRead(JOYSTICK0_SHOOT_HORIZ);

	players[1].naturalVertMove = analogRead(JOYSTICK1_MOVE_VERT);
	players[1].naturalHorMove = analogRead(JOYSTICK1_MOVE_HORIZ);
	players[1].naturalVertShoot = analogRead(JOYSTICK1_SHOOT_VERT);
	players[1].naturalHorShoot = analogRead(JOYSTICK1_SHOOT_HORIZ);

	//initial wins
	players[0].wins = 0;
	players[1].wins = 0;
}

uint32_t lastTime = millis();

/* main game loop. the main game and the menus are all different gameStates
   that are set by the menus, which update on the next loop. input and movement
   updates are dependent on the time since the last update for consistent speeds */
void loop() {
	uint32_t now = millis();
	int dt = now - lastTime;

	// get joystick input from both players
	getInput(dt, &players[0]);
	getInput(dt, &players[1]);

	// check current game state
	switch (gameState) {
		case 0 :
			// main menu
			mainMenu();
			lastTime = millis();
			break;
		case 1 :
			// instructions
			instructionsMenu();
			lastTime = millis();
			break;
		case 2 :
			// main game
			updateCharacters(dt, &players[0]);
			updateCharacters(dt, &players[1]);
			updateProjectiles(dt, &players[0]);
			updateProjectiles(dt, &players[1]);
			checkCollisions(&players[0], &players[1]);
			checkCollisions(&players[1], &players[0]);
			spawnPowerUp(&powerUp);

			// if player1 clicks, enter pause menu
			if (!digitalRead(JOYSTICK0_MOVE_BUTTON)) {
				gameState = 3;
			}

			// win condition, also updates win counter
			if(players[1].health <= 0) {
				players[0].wins++;
				endMenu(0);
			}
			if(players[0].health <= 0) {
				players[1].wins++;
				endMenu(1);
			}

			lastTime = now;
			break;
		case 3:
			// pause menu
			pauseMenu();
			lastTime = millis();
			break;
	}
}