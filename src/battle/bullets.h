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
#include "../json/cJSON.h"

#define TURN_SPEED 			2
#define TURN_THRESHOLD 		3

extern SDL_Texture *getTexture(char *filename);
extern void blitRotated(SDL_Texture *texture, int x, int y, int angle);
extern int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
extern void damageFighter(Entity *f, int damage, long flags);
extern void playBattleSound(int id, int x, int y);
extern long flagsToLong(char *flags);
extern long lookup(char *name);
extern char *readFile(char *filename);
extern float getAngle(int x1, int y1, int x2, int y2);
extern void addMissileEngineEffect(Bullet *b);
extern int mod(int n, int x);
extern void addMissileExplosion(Bullet *b);
extern Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore);

extern Battle battle;
extern Entity *player;
