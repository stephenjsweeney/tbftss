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

#define TURN_SPEED 		0.1
#define TURN_THRESHOLD 	2

#define AVOID_FORCE		500

#include "../common.h"

#include "../json/cJSON.h"

extern Entity *spawnEntity(void);
extern void addSmallExplosion(void);
extern void playBattleSound(int id, int x, int y);
extern float getAngle(int x1, int y1, int x2, int y2);
extern SDL_Texture *getTexture(char *filename);
extern char *readFile(char *filename);
extern char *getFileLocation(char *filename);
extern long flagsToLong(char *flags, int *add);
extern long lookup(char *name);
extern void doAI(void);
extern float getAngle(int x1, int y1, int x2, int y2);
extern float mod(float n, float x);
extern void applyFighterThrust(void);
extern void addLargeEngineEffect(void);
extern int getDistance(int x1, int y1, int x2, int y2);
extern Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore);
extern void addDebris(int x, int y, int amount);
extern void runScriptFunction(char *format, ...);
extern void updateObjective(char *name, int type);
extern char **getFileList(char *dir, int *count);
extern int getJSONValue(cJSON *node, char *name, int defValue);

extern Battle battle;
extern Entity *self;
