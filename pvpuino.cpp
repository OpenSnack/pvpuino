// Steven Boddez & George Antonious, CMPUT 274 Section EA1

#include <Arduino.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS	5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

// joystick/button pin defines
// player 1
#define JOYSTICK0_MOVE_HORIZ 0   // Analog input A0 - horizontal
#define JOYSTICK0_MOVE_VERT  1   // Analog input A1 - vertical
#define JOYSTICK0_MOVE_BUTTON 2  // Digital input pin 2 for the button
#define JOYSTICK0_SHOOT_HORIZ 2
#define JOYSTICK0_SHOOT_VERT 3
// player 2
#define JOYSTICK1_MOVE_HORIZ 4
#define JOYSTICK1_MOVE_VERT 5
#define JOYSTICK1_SHOOT_HORIZ 6
#define JOYSTICK1_SHOOT_VERT 7

#define NUM_PROJECTILES 20

typedef struct {
	float vert;
	float hor;
	float vertSpeed;
	float horSpeed;
	int size;
	int damage;
	int color;
} Projectile;

typedef struct {
	int x;
	int y;
	int length;
	int height;
} Wall;

typedef struct {
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

const int screen_width = 128;
const int screen_height = 160;
const int threshold = 2;
const int playerSize = 8;
const int health_bar_height = 5;
const int default_damage = 5;
const int numPlayers = 2;
const int power_up_size = 8;
Player players[numPlayers];
int gameState = 0;

typedef struct {
	uint32_t timer;
	int x;
	int y;
	int type;
	int onMap;
} Power;

Power powerUp;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// generates a random number with a specified amount of bits
uint32_t randomNumber(int bits) {
	uint32_t value = 0;

	for (int i = 0; i < bits; i++) {
        value = value + ((analogRead(15) % 2) << i);
    }

    return value;
}	

void drawPowerUp(Power* powerUp) {
	switch(powerUp->type) {
		case 0: 
			tft.drawRect(powerUp->x, powerUp->y, power_up_size, power_up_size, ST7735_RED);
			tft.fillRect(powerUp->x, powerUp->y + 3, power_up_size, 2, ST7735_RED);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, power_up_size, ST7735_RED);
			break;
		case 1:
			tft.drawRect(powerUp->x, powerUp->y, power_up_size, power_up_size, ST7735_BLUE);
			tft.fillRect(powerUp->x, powerUp->y + 3, power_up_size, 2, ST7735_BLUE);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, power_up_size, ST7735_BLUE);
			break;
		case 2:
			tft.drawRect(powerUp->x, powerUp->y, power_up_size, power_up_size, ST7735_YELLOW);
			tft.fillRect(powerUp->x, powerUp->y + 3, power_up_size, 2, ST7735_YELLOW);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, power_up_size, ST7735_YELLOW);
			break;
		case 3:
			tft.drawRect(powerUp->x, powerUp->y, power_up_size, power_up_size, ST7735_GREEN);
			tft.fillRect(powerUp->x, powerUp->y + 3, power_up_size, 2, ST7735_GREEN);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, power_up_size, ST7735_GREEN);
			break;
	}
}

void spawnPowerUp(Power* powerUp){
	if(millis() - powerUp->timer > 5000 && powerUp->onMap == 0) {
		if (randomNumber(1)){
			// add while loop to produce random x and ys till it spanws outside of a wall
			int randomX = randomNumber(7);
			int randomY = randomNumber(7);
			powerUp->x = min(randomX, screen_width - power_up_size);
			powerUp->y = constrain(randomY, health_bar_height*2, screen_height - health_bar_height*2 - power_up_size);
			powerUp->type = randomNumber(2);
			powerUp->onMap = 1;
		}
	}

	// redraws power up if it remains active to prevent bullets from
	// drawing over it
	if(powerUp->onMap){
		drawPowerUp(powerUp);
	}
}

void applyPowerUp(Player* player, Power* powerUp){
	switch (powerUp->type) {
		case 0:
			player->health += 30;
			// updates health bar
			if (player->color == ST7735_RED) {
				tft.fillRect(0, screen_height - health_bar_height, screen_width, health_bar_height, ST7735_WHITE);
				tft.fillRect(0, screen_height - health_bar_height, players->health, health_bar_height, ST7735_RED);
			} else if (player->color == ST7735_BLUE){
				tft.fillRect(0, 0, screen_width, health_bar_height, ST7735_WHITE);
				tft.fillRect(0, 0, player->health, health_bar_height, ST7735_BLUE);
			}
			break;
		case 1:
			player->defense = 0;
			break;
		case 2:
			player->damageModifier = 3;
			break;
		case 3:
			player->burstLimit = 0;
			break;
	}

	if (powerUp->type > 0) {
		if (player->color == ST7735_RED) {
			tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height, ST7735_YELLOW);
		} else if (player->color == ST7735_BLUE){
			tft.fillRect(0, health_bar_height, screen_width, health_bar_height, ST7735_YELLOW);
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
	} else if (player->powerUpTimer != -1){
		// set this up so it does not happen on every frame
		powerUpWidth = screen_width - map(powerUpDuration, 0, 5000, 0, screen_width);
		
		if (player->color == ST7735_RED) {
			tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height, ST7735_WHITE);
			tft.fillRect(0, screen_height - health_bar_height*2, powerUpWidth, health_bar_height, ST7735_YELLOW);
		} else if (player->color == ST7735_BLUE){
			tft.fillRect(0, health_bar_height, screen_width, health_bar_height, ST7735_WHITE);
			tft.fillRect(0, health_bar_height, powerUpWidth, health_bar_height, ST7735_YELLOW);
		}

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
			proj->hor = player->x + (float)playerSize/2.0 - (float)proj->size/2.0;
			proj->vert = player->y + (float)playerSize/2.0 - (float)proj->size/2.0;
			float square = (float)abs((vertShoot * vertShoot) + (horShoot * horShoot));
			proj->vertSpeed = vertShoot / (float)sqrt(square) / 8.0;
			proj->horSpeed = horShoot / (float)sqrt(square) / 8.0;
			tft.fillRect(proj->hor, proj->vert, proj->size, proj->size, ST7735_GREEN);
			return 1;
		}
	}
	return 0;
}

void moveProjectile(int dt, Projectile *projectile) {

	tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, tft.Color565(0x00, 0xff, 0xff));
	projectile->hor += projectile->horSpeed * dt;
	projectile->vert += projectile->vertSpeed * dt;

	if(projectile->hor < 0 || projectile->hor > screen_width - projectile->size || projectile->vert < health_bar_height*2 || projectile->vert > screen_height - health_bar_height*2 - projectile->size) {
		// reset projectile for re-use
		projectile->vertSpeed = 0.0;
		projectile->horSpeed = 0.0;
		projectile->hor = -100.0; // offscreen
		projectile->vert = -100.0;
	} else {
		tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, projectile->color);
	}

}

// make it take in pointers
void checkCollisions() {
	// checks if player 2 is hit
	for (int i = 0; i < NUM_PROJECTILES; i++){
		if(players[0].projectiles[i].vertSpeed != 0.0 || players[0].projectiles[i].horSpeed != 0.0){
			if((players[0].projectiles[i].hor + players[0].projectiles[i].size)  > players[1].x 
				&& players[0].projectiles[i].hor < players[1].x + playerSize 
				&& (players[0].projectiles[i].vert + players[0].projectiles[i].size) > players[1].y 
				&& players[0].projectiles[i].vert < players[1].y + playerSize){

				Serial.println("P2 HIT!");
				players[1].health -= players[0].projectiles[i].damage*players[1].defense;
				players[0].projectiles[i].horSpeed = 0.0; // reset projectile
				players[0].projectiles[i].vertSpeed = 0.0;
				tft.fillRect(players[0].projectiles[i].hor, players[0].projectiles[i].vert, players[0].projectiles[i].size, players[0].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);
				players[0].projectiles[i].hor = -100.0; // offscreen
				players[0].projectiles[i].vert = -100.0;
				tft.fillRect(0, 0, screen_width, health_bar_height, ST7735_WHITE);
				tft.fillRect(0, 0, players[1].health, health_bar_height, ST7735_BLUE);

			}
		}
	}

	//checks if player 1 is hit
	for (int i = 0; i < NUM_PROJECTILES; i++){
		if(players[1].projectiles[i].vertSpeed != 0 || players[1].projectiles[i].horSpeed != 0){
			if((players[1].projectiles[i].hor + players[1].projectiles[i].size)  > players[0].x 
				&& players[1].projectiles[i].hor < players[0].x + playerSize 
				&& (players[1].projectiles[i].vert + players[1].projectiles[i].size) > players[0].y 
				&& players[1].projectiles[i].vert < players[0].y + playerSize){

				Serial.println("P2 HIT!");
				players[0].health -= players[1].projectiles[i].damage*players[0].defense;
				players[1].projectiles[i].horSpeed = 0.0; // reset projectile
				players[1].projectiles[i].vertSpeed = 0.0;
				tft.fillRect(players[1].projectiles[i].hor, players[1].projectiles[i].vert, players[1].projectiles[i].size, players[1].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
				players[1].projectiles[i].hor = -100.0; // offscreen
				players[1].projectiles[i].vert = -100.0;
				tft.fillRect(0, screen_height - health_bar_height, screen_width, health_bar_height, ST7735_WHITE);
				tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);
			}
		}
	}

	// this might works
	if (powerUp.onMap == 1) {
		for(int i = 0; i < numPlayers; i++){
			if(players[i].x + playerSize/2 > powerUp.x
				&& players[i].x + playerSize/2 < powerUp.x + power_up_size
				&& players[i].y + playerSize/2 > powerUp.y 
				&& players[i].y + playerSize/2 < powerUp.y + power_up_size){

				tft.fillRect(powerUp.x, powerUp.y, power_up_size, power_up_size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[i].x, players[i].y, playerSize, playerSize, players[i].color);

				// applys power up effect to player
				applyPowerUp(&players[i], &powerUp);

			}
		}
	}
}

void updateCharacters(int dt, Player *player) {
	float newX = player->x;
	float newY = player->y;

	/* -> checks that the joystick moved
	   -> checks that we are at any edge of the screen -> stop moving that direction
	   -> if we are not at a boundary, move the character vertSpeed or horSpeed or both */
	// down movement
	if(player->vertMove > threshold) {
		if(newY > screen_height - player->vertSpeed - playerSize - health_bar_height*2) {
			newY = (float)(screen_height - playerSize - health_bar_height*2.0);
		} else {
			newY += player->vertSpeed;
		}
	}
	// up movement
	if(player->vertMove < -threshold) {
		if(newY < health_bar_height*2 + abs(player->vertSpeed)) {
			newY = (float)health_bar_height*2.0;
		} else {
			newY += player->vertSpeed;
		}
	}
	// right/left movement
	if(player->horMove > threshold) {
		if(newX > screen_width - player->horSpeed - playerSize) {
			newX = (float)(screen_width - playerSize);
		} else {
			newX += player->horSpeed;
		}
	}
	// left movement
	if(player->horMove < -threshold) {
		if(newX < abs(player->horSpeed)) {
			newX = 0.0;
		} else {
			newX += player->horSpeed;
		}
	}

	// draw the player
	// prevents blinking by only redrawing the character if we moved
	if (abs(player->vertMove) > threshold || abs(player->horMove) > threshold) {
		tft.fillRect(player->x, player->y, playerSize, playerSize, tft.Color565(0x00, 0xff, 0xff));
		tft.fillRect(newX, newY, playerSize, playerSize, player->color);
	}

	player->x = newX;
	player->y = newY;

	updatePowerUpState(player);
}

void updateProjectiles(int dt, Player *player) {
	int shooting;
	if(abs(player->vertShoot) > threshold || abs(player->horShoot) > threshold) {
		if(millis() - player->shootTimer > player->burstLimit) {
			shooting = newProjectile(dt, player, 2, default_damage*player->damageModifier);
			player->shootTimer = millis();
		}
	}

	for(int i = 0; i < NUM_PROJECTILES; i++) {
		moveProjectile(dt, &player->projectiles[i]);
	}
	if(shooting) {
			tft.fillRect(player->x, player->y, playerSize, playerSize, player->color);
	}
}

void getInput(int dt) {
	// finds the difference between the current joystick potion and the
	// calibrated center
	players[0].vertMove = analogRead(JOYSTICK0_MOVE_VERT) - players[0].naturalVertMove;
	players[0].horMove = analogRead(JOYSTICK0_MOVE_HORIZ) - players[0].naturalHorMove;
	players[0].vertSpeed = players[0].vertMove * dt / 1000.0 / 8.0;
	players[0].horSpeed = players[0].horMove * dt / 1000.0 / 8.0;

	players[0].vertShoot = analogRead(JOYSTICK0_SHOOT_VERT) - players[0].naturalVertShoot;
	players[0].horShoot = analogRead(JOYSTICK0_SHOOT_HORIZ) - players[0].naturalHorShoot;

	players[1].vertMove = -(analogRead(JOYSTICK1_MOVE_VERT) - players[1].naturalVertMove);
	players[1].horMove = -(analogRead(JOYSTICK1_MOVE_HORIZ) - players[1].naturalHorMove);
	players[1].vertSpeed = players[1].vertMove * dt / 1000 / 8.0;
	players[1].horSpeed = players[1].horMove * dt / 1000 / 8.0;

	players[1].vertShoot = -(analogRead(JOYSTICK1_SHOOT_VERT) - players[1].naturalVertShoot);
	players[1].horShoot = -(analogRead(JOYSTICK1_SHOOT_HORIZ) - players[1].naturalHorShoot);

	// disables player2
	// players[1].vertMove = 0;
	// players[1].horMove = 0;
	// players[1].vertSpeed = 0;
	// players[1].horSpeed = 0;
	// players[1].vertShoot = 0;
	// players[1].horShoot = 0;
}

void mainMenu() {
	gameState = 2;

	// draws main menu screen
	tft.fillScreen(ST7735_BLACK);
	// P
	tft.fillRect(12, 32, 4, 24, ST7735_RED);
	tft.fillRect(16, 32, 4, 4, ST7735_RED);
	tft.fillRect(20, 36, 4, 4, ST7735_RED);
	tft.fillRect(16, 40, 4, 4, ST7735_RED);
	// v
	tft.fillRect(28, 44, 4, 4, ST7735_WHITE);
	tft.fillRect(30, 48, 4, 4, ST7735_WHITE);
	tft.fillRect(32, 52, 4, 4, ST7735_WHITE);
	tft.fillRect(34, 48, 4, 4, ST7735_WHITE);
	tft.fillRect(36, 44, 4, 4, ST7735_WHITE);
	// P
	tft.fillRect(48, 32, 4, 24, ST7735_BLUE);
	tft.fillRect(52, 32, 4, 4, ST7735_BLUE);
	tft.fillRect(56, 36, 4, 4, ST7735_BLUE);
	tft.fillRect(52, 40, 4, 4, ST7735_BLUE);
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

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

	tft.setTextWrap(false);

	tft.setCursor((screen_width - 84)/2 + 28, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("PLAY!");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Instructions");
    //end of drawing

    // infintie loop until an option is selected
    uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt);
		// update postion on menu, updates program state
		if (players[0].vertMove > threshold && gameState != 1) {
			//highlight second button
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 28, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("PLAY!");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Instructions");

			gameState = 1;
		} else if (players[0].vertMove < -threshold && gameState!= 2){
			//highlight first button
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 28, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("PLAY!");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Instructions");
			
			gameState = 2;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			break;
		}

		startTime = currentTime;
	}

}

// need to finish
void instructionsMenu() {
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(true);
	tft.setCursor(0,0);
    tft.setTextColor(ST7735_WHITE);
	tft.print("Instructions coming soon!");
 	
 	// infintie loop until an option is selected
 	uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;
		getInput(dt);

 		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			break;
		}

 		startTime = currentTime;
 	} 

 	gameState = 0;  
}

void initializeGame() {
	// intializes player properties
	for (int i = 0; i < numPlayers; i++){
		players[i].x = (screen_width / 2) - 2;
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
			players[i].projectiles[k].vert = -100.0;
			players[i].projectiles[k].hor = -100.0;
		}
	}

	players[0].y = (3* screen_height / 4) - 2;
	players[1].y = (screen_height / 4) - 2;
	players[0].color = ST7735_RED;
	players[1].color = ST7735_BLUE;
	

	tft.fillScreen(tft.Color565(0x00, 0xff, 0xff));
	// draw initial players
	tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
	tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);

	// height boundaries
	tft.fillRect(0, 0, screen_width, health_bar_height*2, ST7735_WHITE);
	tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height*2, ST7735_WHITE);

	// health bars
	tft.fillRect(0, 0, players[1].health, health_bar_height, ST7735_BLUE);
	tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);

	// draws countdown
	// 3
	tft.fillRect(56, 68, 20, 20, ST7735_BLACK);
	tft.fillRect(56, 72, 16, 4, tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(56, 80, 16, 4, tft.Color565(0x00, 0xff, 0xff));
	delay(1000);

	// 2
	tft.fillRect(56, 68, 20, 20, ST7735_BLACK);
	tft.fillRect(56, 72, 16, 4, tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(60, 80, 16, 4, tft.Color565(0x00, 0xff, 0xff));
	delay(1000);
	
	// 1 
	tft.fillRect(56, 68, 20, 20,  tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(64, 68, 4, 20, ST7735_BLACK);
	delay(1000);

	//go
	tft.fillRect(64, 68, 4, 20,  tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(44, 64, 20, 20,  ST7735_BLACK);
	tft.fillRect(48, 68, 16, 4, tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(48, 72, 4, 8, tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(52, 76, 8, 4, tft.Color565(0x00, 0xff, 0xff));

	tft.fillRect(68, 64, 20, 20,  ST7735_BLACK);
	tft.fillRect(72, 68, 12, 12,  tft.Color565(0x00, 0xff, 0xff));
	delay(300);

	tft.fillRect(40, 64, 66, 20,  tft.Color565(0x00, 0xff, 0xff));
	// finishes drawing countdown

	// enters gameplay state
	powerUp.timer = millis();
	powerUp.onMap = 0;
	gameState = 3;
}

void pauseMenu(){
	gameState = 3;
	int enterTime = millis();
	// draws pause menu
	tft.setTextWrap(false);

	tft.setCursor((screen_width - 84)/2 + 10, 50);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Game Paused");

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);
	tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

	tft.setCursor((screen_width - 84)/2 + 24, 82);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Resume");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Exit to Menu");
    // end of drawing

    // stop multiple presses
    delay(200);

	uint32_t startTime = millis();
	// infinite loop until an option is selected
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt);
		// update postion on menu, updates program state
		if (players[0].vertMove > threshold && gameState != 0) {
			//highlight second button
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);

			tft.setCursor((screen_width - 84)/2 + 24, 82);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Resume");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Exit to Menu");
			
			gameState = 0;
		} else if (players[0].vertMove < -threshold && gameState!= 3){
			//highlight first button
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);

			tft.setCursor((screen_width - 84)/2 + 24, 82);
	    	tft.setTextColor(ST7735_WHITE);
	    	tft.print("Resume");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			gameState = 3;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			break;
		}

	}

	// draws the gameplay screen back on
	if (gameState == 3) {
		tft.fillScreen(tft.Color565(0x00, 0xff, 0xff));
		tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
		tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);

		// height boundaries
		tft.fillRect(0, 0, screen_width, health_bar_height*2, ST7735_WHITE);
		tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height*2, ST7735_WHITE);

		// health bars
		tft.fillRect(0, 0, players[1].health, health_bar_height, ST7735_BLUE);
		tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);


		// updates power up timers
		int timeDiff = millis() - enterTime;

		for (int i = 0; i < numPlayers; i++){
			if (players[i].powerUpTimer != -1) {
				players[i].powerUpTimer += timeDiff;
			}
		}

		powerUp.timer += timeDiff;

	}
}

void endMenu(int playerID) {
	gameState = 2;

	// draws pause menu
	tft.setTextWrap(false);

	tft.setCursor(38, 40);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Game Over\n");
    tft.setCursor(23, 60);
    tft.print("Player ");
    tft.print(playerID + 1);
    tft.print(" Wins!");

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);
	tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

	tft.setCursor((screen_width - 84)/2 + 18, 82);
    tft.setTextColor(ST7735_WHITE);
    tft.print("New Game");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Exit to Menu");

    // win counter
    tft.fillRect(0, screen_height - 16, screen_width, 16, ST7735_BLACK);
    tft.setCursor(5, screen_height - 12);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Wins: ");
    tft.setTextColor(ST7735_RED);
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

		getInput(dt);
		// update postion on menu, updates program state
		if (players[0].vertMove > threshold && gameState != 0) {
			//highlight second button
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);

			tft.setCursor((screen_width - 84)/2 + 18, 82);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("New Game");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Exit to Menu");
			
			gameState = 0;
		} else if (players[0].vertMove < -threshold && gameState!= 2){
			//highlight first button
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);

			tft.setCursor((screen_width - 84)/2 + 18, 82);
	    	tft.setTextColor(ST7735_WHITE);
	    	tft.print("New Game");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			gameState = 2;
		}

		if (!digitalRead(JOYSTICK0_MOVE_BUTTON)){
			break;
		}
	}
}

void setup() {
	Serial.begin(9600);

	tft.initR(INITR_BLACKTAB); 
	digitalWrite(JOYSTICK0_MOVE_BUTTON, HIGH);

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

	// disables player 2
	// players[1].naturalVertMove = 0;
	// players[1].naturalHorMove = 0;
	// players[1].naturalVertShoot = 0;
	// players[1].naturalHorShoot = 0;
}

uint32_t lastTime = millis();

void loop() {
	uint32_t now = millis();
	int dt = now - lastTime;

	getInput(dt);

	// checks current game state
	switch (gameState) {
		case 0 :
			mainMenu();
			lastTime = millis();
			break;
		case 1 :
			instructionsMenu();
			lastTime = millis();
			break;
		case 2 :
			initializeGame();
			lastTime = millis();
			break;
		case 3 :
			updateCharacters(dt, &players[0]);
			updateCharacters(dt, &players[1]);
			updateProjectiles(dt, &players[0]);
			updateProjectiles(dt, &players[1]);
			checkCollisions();
			spawnPowerUp(&powerUp);

			if (!digitalRead(JOYSTICK0_MOVE_BUTTON)) {
				gameState = 4;
			}

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

		case 4:
			pauseMenu();
			lastTime = millis();
			break;
	}

	
}