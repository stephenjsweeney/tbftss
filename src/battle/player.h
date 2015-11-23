/*
Copyright (C) 2015 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "SDL2/SDL.h"

#include "../defs.h"
#include "../structs.h"

#define MAX_SELECTABLE_PLAYERS	8
#define MAX_SELECTABLE_TARGETS	8

extern void fireGuns(Entity *owner);
extern void fireMissile(Entity *owner);
extern void applyFighterThrust(void);
extern void applyFighterBrakes(void);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void addHudMessage(SDL_Color c, char *format, ...);
extern int mod(int n, int x);
extern void playSound(int id);
extern void failMission(void);
extern float getAngle(int x1, int y1, int x2, int y2);
extern int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

extern App app;
extern Battle battle;
extern Colors colors;
extern Entity *player;
extern Entity *self;
extern Game game;
