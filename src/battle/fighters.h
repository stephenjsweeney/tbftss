/*
Copyright (C) 2015-2019 Parallel Realities

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

extern void addDebris(int x, int y, int amount);
extern void addEngineEffect(void);
extern void addHudMessage(SDL_Color c, char *format, ...);
extern void addRandomItem(int x, int y);
extern void addShieldSplinterEffect(Entity *ent);
extern void addSmallExplosion(void);
extern void addSmallFighterExplosion(void);
extern void adjustObjectiveTargetValue(char *name, int type, int amount);
extern void attachRope(void);
extern void checkSuspicionLevel(void);
extern void checkZackariaSuspicionLevel(void);
extern void completeMission(void);
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
extern void playBattleSound(int id, int x, int y);
extern char *readFile(char *filename);
extern void runScriptFunction(char *format, ...);
extern Entity *spawnEntity(void);
extern char **toTypeArray(char *types, int *numTypes);
extern void updateCondition(char *name, int type);
extern void updateObjective(char *name, int type);

extern Battle battle;
extern Colors colors;
extern Entity *player;
extern Entity *self;
extern Game game;
