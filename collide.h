#ifndef COLLIDE_H
#define COLLIDE_H

#include "player.h"

int collide(int x1, int y1, int hor1, int vert1, int x2, int y2, int hor2, int vert2);
void checkCollisions(Player *player1, Player *player2);

#endif