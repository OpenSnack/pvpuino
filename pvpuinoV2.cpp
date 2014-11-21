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
#define JOYSTICK_HORIZ 0   // Analog input A0 - horizontal
#define JOYSTICK_VERT  1   // Analog input A1 - vertical
#define JOYSTICK_BUTTON 9  // Digital input pin 9 for the button
#define BUTTON_SHOOT 3
#define BUTTON_MENU 2

typedef struct {
	float vert;
	float hor;
	float vertSpeed;
	float horSpeed;
	int size;
} Projectile;

const int numProjectiles = 9;

typedef struct {
	float x;
	float y;
	uint32_t color;
	int health;
	int damage;
	int shooting;
	uint32_t shootTimer;
	int horMove;
	int vertMove;
	float horSpeed;
	float vertSpeed;
	float lastHorSpeed;
	float lastVertSpeed;
	Projectile projectiles[numProjectiles];
} Player;

const int screen_width = 128;
const int screen_height = 160;
const int threshold = 2;
const int playerSize = 8;

const int health_bar_height = 5;

int naturalVert;
int naturalHor;
int tempNaturalVert;
int tempNaturalHor;

const int numPlayers = 2;
Player players[numPlayers];

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void newProjectile(int playerID, int size) {

	for(int i = 0; i < numProjectiles; i++) {
		if(players[playerID].projectiles[i].vertSpeed == 0 && players[playerID].projectiles[i].horSpeed == 0) {
			Serial.print("shots fired: ");
			Serial.println(i);
			players[playerID].projectiles[i].hor = players[playerID].x;
			players[playerID].projectiles[i].vert = players[playerID].y;
			players[playerID].projectiles[i].horSpeed = players[playerID].horSpeed;
			players[playerID].projectiles[i].vertSpeed = players[playerID].vertSpeed;
			players[playerID].projectiles[i].size = size;
			tft.fillRect(players[playerID].projectiles[i].hor, 
						players[playerID].projectiles[i].vert, 
						players[playerID].projectiles[i].size, 
						players[playerID].projectiles[i].size, ST7735_GREEN);
			return;
		}
	}
}

void moveProjectile(Projectile *projectile, float horSpeed, float vertSpeed) {

	tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, tft.Color565(0x00, 0xff, 0xff));
	projectile->hor += projectile->horSpeed;
	projectile->vert += projectile->vertSpeed;

	if(projectile->hor < 0 || projectile->hor > screen_width || projectile->vert < 0 || projectile->vert > screen_height) {
		// reset projectile for re-use
		projectile->vertSpeed = 0;
		projectile->horSpeed = 0;
	} else {
		tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, ST7735_BLACK);
	}

}

void checkCollisions() {
	// checks if player 2 is hit
	for (int i = 0; i < numProjectiles; i++){
		if(players[0].projectiles[i].vertSpeed != 0 || players[0].projectiles[i].horSpeed != 0){
			if((players[0].projectiles[i].hor + players[0].projectiles[i].size)  > players[1].x 
				&& players[0].projectiles[i].hor < players[1].x + playerSize 
				&& (players[0].projectiles[i].vert + players[0].projectiles[i].size) > players[1].y 
				&& players[0].projectiles[i].vert < players[1].y + playerSize){

				Serial.println("P2 HIT!");
				players[1].health -= players[0].damage;
				players[0].projectiles[i].horSpeed = 0;
				players[0].projectiles[i].vertSpeed = 0;
				tft.fillRect(players[0].projectiles[i].hor, players[0].projectiles[i].vert, players[0].projectiles[i].size, players[0].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);
				players[0].projectiles[i].hor = -5;
				players[0].projectiles[i].vert = -5;
				tft.fillRect((screen_width - 100) / 2, 10, 100 , health_bar_height, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect((screen_width - 100) / 2, 10, players[1].health , health_bar_height, ST7735_RED);

			}
		}
	}

	//checks if player 1 is hit
	for (int i = 0; i < numProjectiles; i++){
		if(players[1].projectiles[i].vertSpeed != 0 || players[1].projectiles[i].horSpeed != 0){
			if((players[1].projectiles[i].hor + players[1].projectiles[i].size)  > players[0].x 
				&& players[1].projectiles[i].hor < players[0].x + playerSize 
				&& (players[1].projectiles[i].vert + players[1].projectiles[i].size) > players[0].y 
				&& players[1].projectiles[i].vert < players[0].y + playerSize){

				Serial.println("P2 HIT!");
				players[0].health -= players[1].damage;
				players[1].projectiles[i].horSpeed = 0;
				players[1].projectiles[i].vertSpeed = 0;
				tft.fillRect(players[1].projectiles[i].hor, players[1].projectiles[i].vert, players[1].projectiles[i].size, players[1].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
				players[1].projectiles[i].hor = -5;
				players[1].projectiles[i].vert = -5;
				tft.fillRect((screen_width - 100) / 2, screen_height - 15 , 100 , health_bar_height, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect((screen_width - 100) / 2, screen_height - 15, players[0].health , health_bar_height, ST7735_RED);

			}
		}
	}
}

void updateCharacters(int dt, int playerID) {
	float newX = players[playerID].x;
	float newY = players[playerID].y;

	/* -> checks that the joystick moved
	   -> checks that we are at any edge of the screen -> stop moving that direction
	   -> if we are not at a boundary, move the character vertSpeed or horSpeed or both */
	// down movement
	if(players[playerID].vertMove > threshold) {
		if(newY > screen_height - players[playerID].vertSpeed - playerSize) {
			newY = screen_height - playerSize;
		} else {
			newY += (players[playerID].vertSpeed + players[playerID].lastVertSpeed) / 2;
		}
	}
	// up movement
	if(players[playerID].vertMove < -threshold) {
		if(newY < abs(players[playerID].vertSpeed)) {
			newY = 0;
		} else {
			newY += (players[playerID].vertSpeed + players[playerID].lastVertSpeed) / 2;
		}
	}
	// right/left movement
	if(players[playerID].horMove > threshold) {
		if(newX > screen_width - players[playerID].horSpeed - playerSize) {
			newX = screen_width - playerSize;
		} else {
			newX += (players[playerID].horSpeed + players[playerID].lastHorSpeed) / 2;
		}
	}
	// left movement
	if(players[playerID].horMove < -threshold) {
		if(newX < abs(players[playerID].horSpeed)) {
			newX = 0;
		} else {
			newX += (players[playerID].horSpeed + players[playerID].lastHorSpeed) / 2;
		}
	}

	// draw the player
	// prevents blinking by only redrawing the character if we moved
	if (abs(players[playerID].vertMove) > threshold || abs(players[playerID].horMove) > threshold) {
		tft.fillRect(players[playerID].x, players[playerID].y, playerSize, playerSize, tft.Color565(0x00, 0xff, 0xff));
		tft.fillRect(newX, newY, playerSize, playerSize, players[playerID].color);
	}

	players[playerID].x = newX;
	players[playerID].y = newY;
	players[playerID].lastHorSpeed = players[playerID].horSpeed;
	players[playerID].lastVertSpeed = players[playerID].vertSpeed;
}

void updateProjectiles(int dt, int playerID) {
	if(players[playerID].shooting && millis() - players[playerID].shootTimer > 500) {
		newProjectile(playerID, 2);
		players[playerID].shootTimer = millis();
	}

	for(int i = 0; i < numProjectiles; i++) {
		Projectile proj = players[playerID].projectiles[i];
		moveProjectile(&players[playerID].projectiles[i], proj.horSpeed, proj.vertSpeed);
	}
}

void getInput(int dt) {
	// finds the difference between the current joystick potion and the
	// calibrated center
	players[0].vertMove = analogRead(JOYSTICK_VERT) - naturalVert;
	players[0].horMove = analogRead(JOYSTICK_HORIZ) - naturalHor;
	players[0].vertSpeed = players[0].vertMove * dt / 1000 / 8.0;
	players[0].horSpeed = players[0].horMove * dt / 1000 / 8.0;

	players[1].vertMove = analogRead(2) - tempNaturalVert;
	players[1].horMove = analogRead(3) - tempNaturalHor;
	players[1].vertSpeed = players[1].vertMove * dt / 1000 / 8.0;
	players[1].horSpeed = players[1].horMove * dt / 1000 / 8.0;
	
	/*
	// commented out for shooting debug
	// debug
	Serial.print(players[0].vertMove);
	Serial.print("  ");
	Serial.print(players[0].horMove);
	Serial.print("  ");
	Serial.print(players[0].vertSpeed);
	Serial.print("  ");
	Serial.print(players[0].horSpeed);
	Serial.print("       \r");
	*/	

	players[0].shooting = !digitalRead(BUTTON_SHOOT);
	players[1].shooting = !digitalRead(2);

	updateCharacters(dt, 0);
	updateCharacters(dt, 1);
	updateProjectiles(dt, 0);
	updateProjectiles(dt, 1);
	checkCollisions();
}

void setup() {
	Serial.begin(9600);

	tft.initR(INITR_BLACKTAB); 

	// initialize buttons
	pinMode(BUTTON_SHOOT, INPUT);
	digitalWrite(BUTTON_SHOOT, HIGH);
	pinMode(BUTTON_MENU, INPUT);
	digitalWrite(BUTTON_MENU, HIGH);

	// calibrates the center postion of the joystick
	naturalVert = analogRead(JOYSTICK_VERT);
	naturalHor = analogRead(JOYSTICK_HORIZ);

	tempNaturalVert = analogRead(2);
	tempNaturalHor = analogRead(3);

	// intializes player positions
	for (int i = 0; i < numPlayers; i++){
		players[i].x = (screen_width / 2) - 2;
		players[i].lastHorSpeed = 0.0;
		players[i].lastVertSpeed = 0.0;
		players[i].health = 100;
		players[i].damage = 10;
		players[i].shootTimer = millis();
	}

	players[0].y = (3* screen_height / 4) - 2;
	players[1].y = (screen_height / 4) - 2;
	players[0].color = ST7735_RED;
	players[1].color = ST7735_BLUE;
	tft.fillScreen(tft.Color565(0x00, 0xff, 0xff));


	tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
	tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);

	tft.fillRect((screen_width - 100) / 2, screen_height - 15,  players[0].health, health_bar_height, ST7735_RED);
	tft.fillRect((screen_width - 100) / 2, 10, players[1].health , health_bar_height, ST7735_RED);
}

uint32_t lastTime = millis();

void loop() {
	uint32_t now = millis();
	int dt = now - lastTime;

	// stuff we need to implement
	//checkCollisions();

	// getInput -> updateCharacters, updateProjectiles
	getInput(dt);

	lastTime = now;
}