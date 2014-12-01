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
#define JOYSTICK0_SOUND_BUTTON 3
// player 2
#define JOYSTICK1_MOVE_HORIZ 4
#define JOYSTICK1_MOVE_VERT 5
#define JOYSTICK1_SHOOT_HORIZ 6
#define JOYSTICK1_SHOOT_VERT 7

#define SPEAKER_PIN 10
#define NUM_PROJECTILES 20

typedef struct {
	float x;
	float y;
	float vertSpeed;
	float horSpeed;
	int size;
	int damage;
	int color;
} Projectile;

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

typedef struct {
	int x;
	int y;
	int width;
	int height;
} Wall;

typedef struct {
	uint32_t timer;
	int x;
	int y;
	int type;
	int onMap;
} Power;

Wall level0[4] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 160},{128, -20, 30, 200}};
Wall level1[8] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 160},{128, -20, 30, 200},{36, 34, 56, 4},{36, 122, 56, 4},{20, 52, 4, 56},{104, 52, 4, 56}};
Wall level2[10] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 160},{128, -20, 30, 200},{28, 32, 22, 22},{18, 76, 28, 28},{36, 124, 10, 10},{88, 24, 14, 14},{70, 54, 22, 22},{76, 96, 34, 34}};
Wall level3[14] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 160},{128, -20, 30, 200},{62, 22, 4, 12},{62, 48, 4, 12},{62, 74, 4, 12},{62, 100, 4, 12},{62, 126, 4, 12},{10, 78, 12, 4},{34, 78, 12, 4},{58, 78, 12, 4},{82, 78, 12, 4},{106, 78, 12, 4}};
Wall level4[5] = {{-30, -20, 188, 30},{-30, 150, 188, 30},{-30, -20, 30, 160},{128, -20, 30, 200},{14, 74, 100, 14}};
int numWalls[5] = {4, 8, 10, 14, 5};
int wallColors[5] = {};
Wall *currentWalls = level2;
int currentLevel;

const int screen_width = 128;
const int screen_height = 160;
const int threshold = 2;
const int playerSize = 8;
const int health_bar_height = 5;
const int default_damage = 5;
const int numPlayers = 2;
const int powerUpSize = 8;
Player players[numPlayers];
int gameState = 0;
int soundOn = 1;
int soundCounter = 0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

Power powerUp;

void drawWalls(Wall *level, int currentLevel) {
	for(int i = 4; i < numWalls[currentLevel]; i++) {
		tft.fillRect(level[i].x, level[i].y, level[i].width, level[i].height, wallColors[currentLevel]);
	}
}

int collide(int x1, int y1, int hor1, int vert1, int x2, int y2, int hor2, int vert2) {
	if(x1 + hor1 > x2 && x1 < x2 + hor2 && y1 + vert1 > y2 && y1 < y2 + vert2) {
		return 1;
	}
	return 0;
}

void playSound(int type) {
	if(soundOn) {
		switch(type) {
			case 0:
				// main menu sound
				tone(SPEAKER_PIN, 349, 200);
				delay(200);
				tone(SPEAKER_PIN, 440, 100);
				delay(100);
				tone(SPEAKER_PIN, 466, 200);
				break;
			case 1:
				// menu movement
				tone(SPEAKER_PIN, 523, 100);
				break;
			case 2:
				// ok sound
				tone(SPEAKER_PIN, 392, 100);
				delay(100);
				tone(SPEAKER_PIN, 523, 100);
				break;
			case 3:
				// 3, 2, 1
				tone(SPEAKER_PIN, 440, 600);
				break;
			case 4:
				// go
				tone(SPEAKER_PIN, 880, 600);
				break;
			case 5:
				// powerup appeared
				tone(SPEAKER_PIN, 294, 100);
				break;
			case 6:
				// powerup pickup
				// tone(SPEAKER_PIN, 392, 50);
				// delay(50);
				tone(SPEAKER_PIN, 523, 50);
				// delay(50);
				// tone(SPEAKER_PIN, 659, 50);
				break;
			case 7:
				// someone won
				tone(SPEAKER_PIN, 330, 100);
				delay(100);
				tone(SPEAKER_PIN, 349, 100);
				delay(100);
				tone(SPEAKER_PIN, 370, 100);
				delay(100);
				tone(SPEAKER_PIN, 392, 100);
				delay(100);
				tone(SPEAKER_PIN, 415, 100);
				delay(100);
				tone(SPEAKER_PIN, 440, 100);
				delay(100);
				tone(SPEAKER_PIN, 466, 100);
				delay(100);
				tone(SPEAKER_PIN, 494, 100);
				delay(100);
				tone(SPEAKER_PIN, 523, 100);
				delay(200);
				tone(SPEAKER_PIN, 494, 100);
				delay(200);
				tone(SPEAKER_PIN, 523, 200);
				delay(200);
				break;
		}
	}
}

void drawPowerUp(Power* powerUp) {
	switch(powerUp->type) {
		case 0: 
			tft.drawRect(powerUp->x, powerUp->y, powerUpSize, powerUpSize, ST7735_RED);
			tft.fillRect(powerUp->x, powerUp->y + 3, powerUpSize, 2, ST7735_RED);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, powerUpSize, ST7735_RED);
			break;
		case 1:
			tft.drawRect(powerUp->x, powerUp->y, powerUpSize, powerUpSize, ST7735_BLUE);
			tft.fillRect(powerUp->x, powerUp->y + 3, powerUpSize, 2, ST7735_BLUE);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, powerUpSize, ST7735_BLUE);
			break;
		case 2:
			tft.drawRect(powerUp->x, powerUp->y, powerUpSize, powerUpSize, ST7735_YELLOW);
			tft.fillRect(powerUp->x, powerUp->y + 3, powerUpSize, 2, ST7735_YELLOW);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, powerUpSize, ST7735_YELLOW);
			break;
		case 3:
			tft.drawRect(powerUp->x, powerUp->y, powerUpSize, powerUpSize, ST7735_GREEN);
			tft.fillRect(powerUp->x, powerUp->y + 3, powerUpSize, 2, ST7735_GREEN);
			tft.fillRect(powerUp->x + 3, powerUp->y, 2, powerUpSize, ST7735_GREEN);
			break;
	}
}

void spawnPowerUp(Power* powerUp){
	if(millis() - powerUp->timer > 5000 && powerUp->onMap == 0) {
		if (random(2)){
			
			int randomX = random(127-powerUpSize);
			int randomY = random(127-powerUpSize);

			Wall wall;
			int safeWalls = 0;

			for(int i = 0; i < numWalls[currentLevel]; i++) {
				wall = currentWalls[i];
				if (collide(randomX, randomY, powerUpSize, powerUpSize, wall.x, wall.y, wall.width, wall.height)){
					break;
				}
				safeWalls++;
			}

			if (safeWalls == numWalls[currentLevel]){
				powerUp->x = min(randomX, screen_width - powerUpSize);
				powerUp->y = constrain(randomY, health_bar_height*2, screen_height - health_bar_height*2 - powerUpSize);
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

void applyPowerUp(Player* player, Power* powerUp){
	playSound(6);
	switch (powerUp->type) {
		case 0:
			player->health += 30;
			// updates health bar
			if (player->ID == 0) {
				tft.fillRect(0, screen_height - health_bar_height, screen_width, health_bar_height, ST7735_BLACK);
				tft.fillRect(0, screen_height - health_bar_height, player->health, health_bar_height, ST7735_RED);
			} else if (player->ID == 1){
				tft.fillRect(0, 0, screen_width, health_bar_height, ST7735_BLACK);
				tft.fillRect(0, 0, player->health, health_bar_height, ST7735_CYAN);
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
			tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height, ST7735_YELLOW);
		} else if (player->ID == 1){
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
		if(player->ID == 0) {
			player->color = ST7735_RED;
		} else if(player->ID == 1) {
			player->color = ST7735_CYAN;
		}
		if (player->ID == 0) {
			tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height, wallColors[currentLevel]);
		} else if (player->ID == 1) {
			tft.fillRect(0, health_bar_height, screen_width, health_bar_height, wallColors[currentLevel]);
		}
	} else if (player->powerUpTimer != -1){
		// set this up so it does not happen on every frame
		powerUpWidth = screen_width - map(powerUpDuration, 0, 5000, 0, screen_width);
		
		if (player->ID == 0) {
			tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height, wallColors[currentLevel]);
			tft.fillRect(0, screen_height - health_bar_height*2, powerUpWidth, health_bar_height, ST7735_YELLOW);
		} else if (player->ID == 1) {
			tft.fillRect(0, health_bar_height, screen_width, health_bar_height, wallColors[currentLevel]);
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
			proj->x = player->x + (float)playerSize/2.0 - (float)proj->size/2.0;
			proj->y = player->y + (float)playerSize/2.0 - (float)proj->size/2.0;
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

void checkCollisions(Player *player1, Player *player2) {
	// checks if player1 is hit
	for (int i = 0; i < NUM_PROJECTILES; i++){
		Projectile *proj = &player2->projectiles[i];

		if(proj->vertSpeed != 0.0 || proj->horSpeed != 0.0) {
			if(collide(player1->x, player1->y, playerSize, playerSize, proj->x, proj->y, proj->size, proj->size)) {
				player1->health -= proj->damage * player1->defense;
				proj->horSpeed = 0.0; // reset projectile
				proj->vertSpeed = 0.0;
				tft.fillRect(proj->x, proj->y, proj->size, proj->size, ST7735_BLACK);
				tft.fillRect(player1->x, player1->y, playerSize, playerSize, player1->color);
				proj->x = -100.0; // offscreen
				proj->y = -100.0;
				if (player1->ID == 0) {
					tft.fillRect(0, screen_height - health_bar_height, screen_width, health_bar_height, ST7735_BLACK);
					tft.fillRect(0, screen_height - health_bar_height, player1->health, health_bar_height, ST7735_RED);
				} else if (player1->ID == 1) {
					tft.fillRect(0, 0, screen_width, health_bar_height, ST7735_BLACK);
					tft.fillRect(0, 0, player1->health, health_bar_height, ST7735_CYAN);
				}
			}
		}
	}

	if (powerUp.onMap == 1) {
		if(collide(player1->x, player1->y, playerSize, playerSize, powerUp.x, powerUp.y, powerUpSize, powerUpSize)) {
			tft.fillRect(powerUp.x, powerUp.y, powerUpSize, powerUpSize, ST7735_BLACK);
			tft.fillRect(player1->x, player1->y, playerSize, playerSize, player1->color);

			// applys power up effect to player
			applyPowerUp(player1, &powerUp);
		}
	}
}

void updateCharacters(int dt, Player *player) {
	float newX = player->x;
	float newY = player->y;
	
	Wall wall;
	int touching = 0;

	// down movement
	if(player->vertMove > threshold) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newY + playerSize > wall.y - player->vertSpeed && newY + playerSize < wall.y + wall.height - player->vertSpeed) {
				if(newX + playerSize > wall.x && newX < wall.x + wall.width) {
					newY = (float)(wall.y - playerSize);
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
	if(player->vertMove < -threshold) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newY < wall.y + wall.height - player->vertSpeed && newY > wall.y - player->vertSpeed) {
				if(newX + playerSize > wall.x && newX < wall.x + wall.width) {
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
	if(player->horMove > threshold) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newX + playerSize > wall.x - player->horSpeed && newX + playerSize < wall.x + wall.width - player->horSpeed) {
				if(newY + playerSize > wall.y && newY < wall.y + wall.height) {
					newX = (float)(wall.x - playerSize);
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
	if(player->horMove < -threshold) {
		for(int i = 0; i < numWalls[currentLevel]; i++) {
			wall = currentWalls[i];
			if(newX < wall.x + wall.width - player->horSpeed && newX > wall.x - player->horSpeed) {
				if(newY + playerSize > wall.y && newY < wall.y + wall.height) {
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
	if (abs(player->vertMove) > threshold || abs(player->horMove) > threshold) {
		tft.fillRect(player->x, player->y, playerSize, playerSize, ST7735_BLACK);
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

	if(!digitalRead(JOYSTICK0_SOUND_BUTTON)) {
		soundOn = !soundOn;
		delay(200);
	}
}

void initializeGame() {
	// intializes player properties
	for (int i = 0; i < numPlayers; i++){
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

	players[0].color = ST7735_RED;
	players[1].color = ST7735_CYAN;

	tft.fillScreen(ST7735_BLACK);

	// load chosen level
	switch (currentLevel) {
		case 0:
			currentWalls = level0;
			players[0].x = screen_width/2 - playerSize/2;
			players[0].y = 3*screen_height/4;
			players[1].x = screen_width/2 - playerSize/2;
			players[1].y = screen_height/4 - playerSize;
			break;
		case 1:
			currentWalls = level1;
			players[0].x = screen_width/2 - playerSize/2;
			players[0].y = screen_height - 15 - playerSize;
			players[1].x = screen_width/2 - playerSize/2;
			players[1].y = 15;
			break;
		case 2:
			currentWalls = level2;
			players[0].x = screen_width/2 - playerSize/2;
			players[0].y = 3*screen_height/4;
			players[1].x = screen_width/2 - playerSize/2;
			players[1].y = screen_height/4 - playerSize;
			break;
		case 3:
			currentWalls = level3;
			players[0].x = screen_width - 20 - playerSize;
			players[0].y = screen_height - 40 - playerSize;
			players[1].x = 20;
			players[1].y = 40;
			break;
		case 4:
			currentWalls = level4;
			players[0].x = screen_width/2 - playerSize/2;
			players[0].y = screen_height - 40 - playerSize;
			players[1].x = screen_width/2 - playerSize/2;
			players[1].y = 40;
			break;
	}

	drawWalls(currentWalls, currentLevel);

	// draw initial players
	tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
	tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);

	// height boundaries
	tft.fillRect(0, 0, screen_width, health_bar_height*2, wallColors[currentLevel]);
	tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height*2, wallColors[currentLevel]);

	// health bars
	tft.fillRect(0, 0, players[1].health, health_bar_height, ST7735_CYAN);
	tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);

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
	tft.setCursor((screen_width - 84)/2 + 10, 12);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Select Map");

    tft.fillRect((screen_width - 84)/2, 32, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 56, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 80, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 104, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 128, 84, 16, ST7735_WHITE);

	tft.setTextWrap(false);

	tft.setCursor((screen_width - 84)/2 + 24, 36);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Simple");

    tft.setCursor((screen_width - 84)/2 + 27, 60);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Arena");

    tft.setCursor((screen_width - 84)/2 + 27, 84);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Boxes");

    tft.setCursor((screen_width - 84)/2 + 24, 108);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Plus++");

    tft.setCursor((screen_width - 84)/2 + 12, 132);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Final Dest.");

	// infinte loop until an option is selected
 	uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;
		getInput(dt);

		if (players[0].vertMove < -threshold && mapID == 1) {
			playSound(1);
			mapID = 0;

			tft.fillRect((screen_width - 84)/2, 32, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 56, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 56, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 24, 36);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Simple");

		    tft.setCursor((screen_width - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Arena");

		    delay(200);

		} else if ((players[0].vertMove > threshold && mapID == 0) || (players[0].vertMove < -threshold && mapID == 2)) {
			playSound(1);
			mapID = 1;

			tft.fillRect((screen_width - 84)/2, 32, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 32, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 56, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 80, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 80, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 24, 36);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Simple");

			tft.setCursor((screen_width - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Arena");

		    tft.setCursor((screen_width - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Boxes");

		    delay(200);

		} else if ((players[0].vertMove > threshold && mapID == 1) || (players[0].vertMove < -threshold && mapID == 3)) {
			playSound(1);
			mapID = 2;

			tft.fillRect((screen_width - 84)/2, 56, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 56, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 80, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 104, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 104, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 27, 60);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Arena");

			tft.setCursor((screen_width - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Boxes");

		    tft.setCursor((screen_width - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Plus++");

		    delay(200);

		} else if ((players[0].vertMove > threshold && mapID == 2) || (players[0].vertMove < -threshold && mapID == 4)) {
			playSound(1);
			mapID = 3;

			tft.fillRect((screen_width - 84)/2, 80, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 80, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 104, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 128, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 128, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 27, 84);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Boxes");

			tft.setCursor((screen_width - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Plus++");

		    tft.setCursor((screen_width - 84)/2 + 12, 132);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Final Dest.");

		    delay(200);

		} else if (players[0].vertMove > threshold && mapID == 3) {
			playSound(1);
			mapID = 4;

			tft.fillRect((screen_width - 84)/2, 128, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 104, 84, 16, ST7735_BLACK);
			tft.drawRect((screen_width - 84)/2, 104, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 24, 108);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Plus++");

			tft.setCursor((screen_width - 84)/2 + 12, 132);
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

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
	tft.drawRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

	tft.setTextWrap(false);

	tft.setCursor((screen_width - 84)/2 + 28, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("PLAY!");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Instructions");
    playSound(0);
    //end of drawing

    // infintie loop until an option is selected
    uint32_t startTime = millis();
	while(1) {
		uint32_t currentTime = millis();
		uint32_t dt = currentTime - startTime;

		getInput(dt);
		// update postion on menu, updates program state
		if (players[0].vertMove > threshold && menuSelection != 0) {
			playSound(1);
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

			menuSelection = 0;
		} else if (players[0].vertMove < -threshold && menuSelection != 1){
			playSound(1);
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

	tft.setCursor((screen_width - 84)/2 + 10, 50);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Game Paused");

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
	tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);

	tft.setCursor((screen_width - 84)/2 + 24, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("Resume");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
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

		getInput(dt);
		// update postion on menu, updates program state
		if (players[0].vertMove > threshold && gameState != 0) {
			playSound(1);
			//highlight second button
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 24, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("Resume");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			gameState = 0;
		} else if (players[0].vertMove < -threshold && gameState!= 2){
			playSound(1);
			//highlight first button
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);

			tft.setCursor((screen_width - 84)/2 + 24, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("Resume");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
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
		tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
		tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);

		// height boundaries
		tft.fillRect(0, 0, screen_width, health_bar_height*2, ST7735_WHITE);
		tft.fillRect(0, screen_height - health_bar_height*2, screen_width, health_bar_height*2, wallColors[currentLevel]);

		// health bars
		tft.fillRect(0, 0, players[1].health, health_bar_height, ST7735_CYAN);
		tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);

		// walls
		drawWalls(currentWalls, currentLevel);

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

	tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);
	tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);

	tft.setCursor((screen_width - 84)/2 + 18, 82);
    tft.setTextColor(ST7735_BLACK);
    tft.print("New Game");

    tft.setCursor((screen_width - 84)/2 + 6, 106);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Exit to Menu");

    // win counter
    tft.fillRect(0, screen_height - 16, screen_width, 16, ST7735_WHITE);
    tft.setCursor(5, screen_height - 12);
    tft.setTextColor(ST7735_BLACK);
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
		if (players[0].vertMove > threshold && menuSelection != 0) {
			playSound(1);
			//highlight second button
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_BLACK);
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 18, 82);
		    tft.setTextColor(ST7735_WHITE);
		    tft.print("New Game");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
		    tft.setTextColor(ST7735_BLACK);
		    tft.print("Exit to Menu");
			
			menuSelection = 0;
		} else if (players[0].vertMove < -threshold && menuSelection != 1){
			playSound(1);
			//highlight first button
			tft.fillRect((screen_width - 84)/2, 102, 84, 16, ST7735_BLACK);
			tft.fillRect((screen_width - 84)/2, 78, 84, 16, ST7735_WHITE);

			tft.setCursor((screen_width - 84)/2 + 18, 82);
	    	tft.setTextColor(ST7735_BLACK);
	    	tft.print("New Game");

		    tft.setCursor((screen_width - 84)/2 + 6, 106);
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

void setup() {
	Serial.begin(9600);
	randomSeed(analogRead(15));
	wallColors[0] = tft.Color565(0xFF, 0xFF, 0xFF);
	wallColors[1] = tft.Color565(0xFF, 0x00, 0xCC);
	wallColors[2] = tft.Color565(0x00, 0xFF, 0x00);
	wallColors[3] = tft.Color565(0xFF, 0x8D, 0x00);
	wallColors[4] = tft.Color565(0x00, 0x00, 0xFF);

	tft.initR(INITR_BLACKTAB); 
	digitalWrite(JOYSTICK0_MOVE_BUTTON, HIGH);
	digitalWrite(JOYSTICK0_SOUND_BUTTON, HIGH);

	for(int i = 0; i < numPlayers; i++) {
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
			updateCharacters(dt, &players[0]);
			updateCharacters(dt, &players[1]);
			updateProjectiles(dt, &players[0]);
			updateProjectiles(dt, &players[1]);
			checkCollisions(&players[0], &players[1]);
			checkCollisions(&players[1], &players[0]);
			spawnPowerUp(&powerUp);

			if (!digitalRead(JOYSTICK0_MOVE_BUTTON)) {
				gameState = 3;
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
		case 3:
			pauseMenu();
			lastTime = millis();
			break;
	}
}