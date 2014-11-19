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
Projectile P1Projectiles[numProjectiles];

const int screen_width = 128;
const int screen_height = 160;
const int threshold = 2;
const int playerSize = 8;

int naturalVert;
int naturalHor;
float x = (screen_width / 2) - 2;
float y = (screen_height / 2) - 2;
int shootTimer = millis();
float lastHorSpeed = 0.0;
float lastVertSpeed = 0.0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void newProjectile(float x, float y, float horSpeed, float vertSpeed, int size) {
	for(int i = 0; i < numProjectiles; i++) {
		if(P1Projectiles[i].vertSpeed == 0 && P1Projectiles[i].horSpeed == 0) {
			P1Projectiles[i].hor = x;
			P1Projectiles[i].vert = y;
			P1Projectiles[i].horSpeed = horSpeed;
			P1Projectiles[i].vertSpeed = vertSpeed;
			P1Projectiles[i].size = size;
			tft.fillRect(P1Projectiles[i].hor, P1Projectiles[i].vert, P1Projectiles[i].size, P1Projectiles[i].size, ST7735_GREEN);
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

// void checkCollisions() {

// }

void updateCharacters(int dt, int vertMove, int horMove, float vertSpeed, float horSpeed) {
	float newX = x;
	float newY = y;

	/* -> checks that the joystick moved
	   -> checks that we are at any edge of the screen -> stop moving that direction
	   -> if we are not at a boundary, move the character vertSpeed or horSpeed or both */
	// down movement
	if(vertMove > threshold) {
		if(newY > screen_height - vertSpeed - playerSize) {
			newY = screen_height - playerSize;
		} else {
			newY += (vertSpeed + lastVertSpeed) / 2;
		}
	}
	// up movement
	if(vertMove < -threshold) {
		if(newY < abs(vertSpeed)) {
			newY = 0;
		} else {
			newY += (vertSpeed + lastVertSpeed) / 2;
		}
	}
	// right/left movement
	if(horMove > threshold) {
		if(newX > screen_width - horSpeed - playerSize) {
			newX = screen_width - playerSize;
		} else {
			newX += (horSpeed + lastHorSpeed) / 2;
		}
	}
	// left movement
	if(horMove < -threshold) {
		if(newX < abs(horSpeed)) {
			newX = 0;
		} else {
			newX += (horSpeed + lastHorSpeed) / 2;
		}
	}

	// draw the player
	// prevents blinking by only redrawing the character if we moved
	if (abs(vertMove) > threshold || abs(horMove) > threshold) {
		tft.fillRect(x, y, playerSize, playerSize, tft.Color565(0x00, 0xff, 0xff));
		tft.fillRect(newX, newY, playerSize, playerSize, ST7735_RED);
	}

	x = newX;
	y = newY;
	lastHorSpeed = horSpeed;
	lastVertSpeed = vertSpeed;
}

void updateProjectiles(int dt, float vertSpeed, float horSpeed, int shooting) {
	if(shooting && millis() - shootTimer > 500) {
		newProjectile(x, y, horSpeed, vertSpeed, 2);
		shootTimer = millis();
	}

	for(int i = 0; i < numProjectiles; i++) {
		Projectile proj = P1Projectiles[i];
		moveProjectile(&P1Projectiles[i], proj.horSpeed, proj.vertSpeed);
	}
}

void getInput(int dt) {
	// finds the difference between the current joystick potion and the
	// calibrated center
	int vertMove = analogRead(JOYSTICK_VERT) - naturalVert;
	int horMove = analogRead(JOYSTICK_HORIZ) - naturalHor;
	float vertSpeed = vertMove * dt / 1000 / 8.0;
	float horSpeed = horMove * dt / 1000 / 8.0;

	// debug
	Serial.print(vertMove);
	Serial.print("  ");
	Serial.print(horMove);
	Serial.print("  ");
	Serial.print(vertSpeed);
	Serial.print("  ");
	Serial.print(horSpeed);
	Serial.print("       \r");


	int shooting = !digitalRead(BUTTON_SHOOT);

	updateCharacters(dt, vertMove, horMove, vertSpeed, horSpeed);
	updateProjectiles(dt, vertSpeed, horSpeed, shooting);
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

	tft.fillScreen(tft.Color565(0x00, 0xff, 0xff));
	tft.fillRect(x, y, playerSize, playerSize, ST7735_RED);
}

int lastTime = millis();
void loop() {
	int now = millis();
	int dt = now - lastTime;

	// stuff we need to implement
	//checkCollisions();

	// getInput -> updateCharacters, updateProjectiles
	getInput(dt);

	lastTime = now;
}