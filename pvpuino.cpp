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
#define JOYSTICK_MOVE_HORIZ 0   // Analog input A0 - horizontal
#define JOYSTICK_MOVE_VERT  1   // Analog input A1 - vertical
#define JOYSTICK_MOVE_BUTTON 9  // Digital input pin 9 for the button
#define JOYSTICK_SHOOT_HORIZ 2
#define JOYSTICK_SHOOT_VERT 3

#define NUM_PROJECTILES 9

typedef struct {
	float vert;
	float hor;
	float vertSpeed;
	float horSpeed;
	int size;
	int damage;
} Projectile;

typedef struct {
	float x;
	float y;
	uint32_t color;
	int health;
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
	Projectile projectiles[NUM_PROJECTILES];
} Player;

const int screen_width = 128;
const int screen_height = 160;
const int threshold = 2;
const int playerSize = 8;
const int health_bar_height = 5;

const int numPlayers = 2;
Player players[numPlayers];

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void newProjectile(int dt, Player *player, int size, int damage) {
	for(int i = 0; i < NUM_PROJECTILES; i++) {
		Projectile *proj = &player->projectiles[i];
		if(proj->vertSpeed == 0 && proj->horSpeed == 0) {
			proj->hor = player->x;
			proj->vert = player->y;
			proj->vertSpeed = player->vertShoot / sqrt((player->vertShoot * player->vertShoot) + (player->horShoot * player->horShoot)) / 8.0;
			proj->horSpeed = player->horShoot / sqrt((player->vertShoot * player->vertShoot) + (player->horShoot * player->horShoot)) / 8.0;
			proj->size = size;
			proj->damage = damage;
			tft.fillRect(proj->hor, proj->vert, proj->size, proj->size, ST7735_GREEN);
			return;
		}
	}
}

void moveProjectile(int dt, Projectile *projectile) {

	tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, tft.Color565(0x00, 0xff, 0xff));
	projectile->hor += projectile->horSpeed * dt;
	projectile->vert += projectile->vertSpeed * dt;

	if(projectile->hor < 0 || projectile->hor > screen_width - projectile->size || projectile->vert < health_bar_height*2 || projectile->vert > screen_height - health_bar_height*2 - projectile->size) {
		// reset projectile for re-use
		projectile->vertSpeed = 0;
		projectile->horSpeed = 0;
		projectile->hor = -100; // offscreen
		projectile->vert = -100;
	} else {
		tft.fillRect((int)projectile->hor, (int)projectile->vert, projectile->size, projectile->size, ST7735_BLACK);
	}

}

void checkCollisions() {
	// checks if player 2 is hit
	for (int i = 0; i < NUM_PROJECTILES; i++){
		if(players[0].projectiles[i].vertSpeed != 0 || players[0].projectiles[i].horSpeed != 0){
			if((players[0].projectiles[i].hor + players[0].projectiles[i].size)  > players[1].x 
				&& players[0].projectiles[i].hor < players[1].x + playerSize 
				&& (players[0].projectiles[i].vert + players[0].projectiles[i].size) > players[1].y 
				&& players[0].projectiles[i].vert < players[1].y + playerSize){

				Serial.println("P2 HIT!");
				players[1].health -= players[0].projectiles[i].damage;
				players[0].projectiles[i].horSpeed = 0; // reset projectile
				players[0].projectiles[i].vertSpeed = 0;
				tft.fillRect(players[0].projectiles[i].hor, players[0].projectiles[i].vert, players[0].projectiles[i].size, players[0].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[1].x, players[1].y, playerSize, playerSize, players[1].color);
				players[0].projectiles[i].hor = -100; // offscreen
				players[0].projectiles[i].vert = -100;
				tft.fillRect(0, 0, screen_width, health_bar_height, tft.Color565(0x00, 0xff, 0xff));
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
				players[0].health -= players[1].projectiles[i].damage;
				players[1].projectiles[i].horSpeed = 0; // reset projectile
				players[1].projectiles[i].vertSpeed = 0;
				tft.fillRect(players[1].projectiles[i].hor, players[1].projectiles[i].vert, players[1].projectiles[i].size, players[1].projectiles[i].size, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(players[0].x, players[0].y, playerSize, playerSize, players[0].color);
				players[1].projectiles[i].hor = -100; // offscreen
				players[1].projectiles[i].vert = -100;
				tft.fillRect(0, screen_height - health_bar_height, screen_width, health_bar_height, tft.Color565(0x00, 0xff, 0xff));
				tft.fillRect(0, screen_height - health_bar_height, players[0].health, health_bar_height, ST7735_RED);
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
			newY = screen_height - playerSize - health_bar_height*2;
		} else {
			newY += player->vertSpeed;
		}
	}
	// up movement
	if(player->vertMove < -threshold) {
		if(newY < health_bar_height*2 + abs(player->vertSpeed)) {
			newY = health_bar_height*2;
		} else {
			newY += player->vertSpeed;
		}
	}
	// right/left movement
	if(player->horMove > threshold) {
		if(newX > screen_width - player->horSpeed - playerSize) {
			newX = screen_width - playerSize;
		} else {
			newX += player->horSpeed;
		}
	}
	// left movement
	if(player->horMove < -threshold) {
		if(newX < abs(player->horSpeed)) {
			newX = 0;
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
}

void updateProjectiles(int dt, Player *player) {
	if(abs(player->vertShoot) > threshold || abs(player->horShoot > threshold)) {
		if(millis() - player->shootTimer > 500) {
			newProjectile(dt, player, 2, 10);
			player->shootTimer = millis();
		}
	}

	for(int i = 0; i < NUM_PROJECTILES; i++) {
		moveProjectile(dt, &player->projectiles[i]);
	}
}

void getInput(int dt) {
	// finds the difference between the current joystick potion and the
	// calibrated center
	players[0].vertMove = analogRead(JOYSTICK_MOVE_VERT) - players[0].naturalVertMove;
	players[0].horMove = analogRead(JOYSTICK_MOVE_HORIZ) - players[0].naturalHorMove;
	players[0].vertSpeed = players[0].vertMove * dt / 1000 / 8.0;
	players[0].horSpeed = players[0].horMove * dt / 1000 / 8.0;

	players[0].vertShoot = analogRead(JOYSTICK_SHOOT_VERT) - players[0].naturalVertShoot;
	players[0].horShoot = analogRead(JOYSTICK_SHOOT_HORIZ) - players[0].naturalHorShoot;

	// players[1].vertMove = analogRead(2) - players[1].naturalVertMove;
	// players[1].horMove = analogRead(3) - players[1].naturalHorMove;
	// players[1].vertSpeed = players[1].vertMove * dt / 1000 / 8.0;
	// players[1].horSpeed = players[1].horMove * dt / 1000 / 8.0;
	
	// just for meow
	players[1].vertMove = 0;
	players[1].horMove = 0;
	players[1].vertSpeed = 0;
	players[1].horSpeed = 0;
	players[1].vertShoot = 0;
	players[1].horShoot = 0;

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
}

void setup() {
	Serial.begin(9600);

	tft.initR(INITR_BLACKTAB); 

	// calibrates the center postion of the joysticks
	players[0].naturalVertMove = analogRead(JOYSTICK_MOVE_VERT);
	players[0].naturalHorMove = analogRead(JOYSTICK_MOVE_HORIZ);
	players[0].naturalVertShoot = analogRead(JOYSTICK_SHOOT_VERT);
	players[0].naturalHorShoot = analogRead(JOYSTICK_SHOOT_HORIZ);

	// just for meow
	players[1].naturalVertMove = 0;
	players[1].naturalHorMove = 0;
	players[1].naturalVertShoot = 0;
	players[1].naturalHorShoot = 0;

	// intializes player positions
	for (int i = 0; i < numPlayers; i++){
		players[i].x = (screen_width / 2) - 2;
		players[i].horSpeed = 0.0;
		players[i].vertSpeed = 0.0;
		players[i].health = 128;
		players[i].shootTimer = millis();
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
}

uint32_t lastTime = millis();

void loop() {
	uint32_t now = millis();
	int dt = now - lastTime;

	getInput(dt);
	updateCharacters(dt, &players[0]);
	updateCharacters(dt, &players[1]);
	updateProjectiles(dt, &players[0]);
	updateProjectiles(dt, &players[1]);
	checkCollisions();

	lastTime = now;
}