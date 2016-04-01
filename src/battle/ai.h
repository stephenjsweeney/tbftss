/*
Copyright (C) 2015-2016 Parallel Realities

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

#include "../common.h"

#define AI_EVADE		0
#define AI_FALLBACK		1
#define AI_HUNT			2

#define TURN_SPEED 		4
#define TURN_THRESHOLD 	2

extern float mod(float n, float x);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void fireGuns(Entity *owner);
extern void fireMissile(Entity *owner);
extern void fireRocket(Entity *owner);
extern float getAngle(int x1, int y1, int x2, int y2);
extern void applyFighterThrust(void);
extern void applyFighterBrakes(void);
extern void addHudMessage(SDL_Color c, char *format, ...);
extern Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore);
extern char *getTranslatedString(char *string);
extern Entity *spawnMine(void);

extern Battle battle;
extern Colors colors;
extern Dev dev;
extern Entity *self;
extern Entity *player;
