#ifndef DRAW_H
#define DRAW_H

#include "player.h"
#include "projectile.h"

// standard U of A library settings, assuming Atmel Mega SPI pins
#define SD_CS	5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160
#define HEALTH_BAR_HEIGHT 5
#define DEFAULT_DAMAGE 5

typedef struct {
	int x;
	int y;
	int width;
	int height;
} Wall;

extern Adafruit_ST7735 tft;

extern Wall level0[4];
extern Wall level1[8];
extern Wall level2[10];
extern Wall level3[14];
extern Wall level4[5];
extern int numWalls[5];
extern int wallColors[5];
extern Wall *currentWalls;
extern int currentLevel;

void drawWalls(Wall *level, int currentLevel);
int newProjectile(int dt, Player *player, int size, int damage);
void moveProjectile(int dt, Projectile *proj);
void updateCharacters(int dt, Player *player);
void updateProjectiles(int dt, Player *player);

#endif