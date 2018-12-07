/*
Copyright (C) 2015-2018 Parallel Realities

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

#include "../json/cJSON.h"

#define INITIAL_BULLET_DRAW_CAPACITY	32
#define MISSILE_LIFE 					(FPS * 30)
#define TURN_SPEED 						2
#define TURN_THRESHOLD 					3

extern void addBulletHitEffect(int x, int y, int r, int g, int b);
extern void addMissileEngineEffect(Bullet *b);
extern void addMissileExplosion(Bullet *b);
extern void awardTrophy(char *id);
extern void blitRotated(AtlasImage *atlasImage, int x, int y, float angle);
extern int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
extern void damageFighter(Entity *e, int damage, long flags);
extern long flagsToLong(char *flags, int *add);
extern Entity **getAllEntsInRadius(int x, int y, int radius, Entity *ignore);
extern Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore);
extern float getAngle(int x1, int y1, int x2, int y2);
extern AtlasImage *getAtlasImage(char *filename);
extern int getDistance(int x1, int y1, int x2, int y2);
extern int isOnBattleScreen(int x, int y, int w, int h);
extern long lookup(char *name);
extern float mod(float n, float x);
extern void playBattleSound(int id, int x, int y);
extern void playSound(int id);
extern char *readFile(char *filename);
extern void *resize(void *array, int oldSize, int newSize);
extern void setAtlasColor(int r, int g, int b, int a);

extern App app;
extern Battle battle;
extern Entity *player;
