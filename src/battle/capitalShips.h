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

#define TURN_SPEED 		0.1
#define TURN_THRESHOLD 	2

extern void addDebris(int x, int y, int amount);
extern void addLargeEngineEffect(void);
extern void addLargeExplosion(void);
extern void addMessageBox(char *title, char *body, int type);
extern void addSmallExplosion(void);
extern void applyFighterThrust(void);
extern void doAI(void);
extern long flagsToLong(char *flags, int *add);
extern Entity **getAllEntsInRadius(int x, int y, int radius, Entity *ignore);
extern float getAngle(int x1, int y1, int x2, int y2);
extern AtlasImage *getAtlasImage(char *filename);
extern int getDistance(int x1, int y1, int x2, int y2);
extern char **getFileList(char *dir, int *count);
extern int getJSONValue(cJSON *node, char *name, int defValue);
extern char *getJSONValueStr(cJSON *node, char *name, char *defValue);
extern long lookup(char *name);
extern float mod(float n, float x);
extern void playBattleSound(int id, int x, int y);
extern char *readFile(char *filename);
extern void runScriptFunction(char *format, ...);
extern Entity *spawnEntity(void);
extern char **toTypeArray(char *types, int *numTypes);
extern void updateCondition(char *name, int type);
extern void updateObjective(char *name, int type);

extern Battle battle;
extern Entity *player;
extern Entity *self;
