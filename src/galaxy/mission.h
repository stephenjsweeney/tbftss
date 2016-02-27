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

#include "time.h"

#include "../json/cJSON.h"

extern long lookup(char *name);
extern char *readFile(char *filename);
extern SDL_Texture *getTexture(char *filename);
extern Entity *spawnFighter(char *name, int x, int y, int side);
extern Entity *spawnCapitalShip(char *name, int x, int y, int side);
extern void startSectionTransition(void);
extern void endSectionTransition(void);
extern void playMusic(char *filename);
extern void stopMusic(void);
extern void initPlayer(void);
extern long flagsToLong(char *flags, int *add);
extern Entity *spawnWaypoint(void);
extern void activateNextWaypoint(void);
extern void selectWidget(const char *name, const char *group);
extern Entity *spawnExtractionPoint(void);
extern Entity *spawnItem(char *type);
extern void failIncompleteObjectives(void);
extern void completeConditions(void);
extern void retreatEnemies(void);
extern void initScript(cJSON *missionJSON);
extern char *getFileLocation(char *filename);
extern void updateCapitalShipComponentProperties(Entity *parent);
extern void countNumEnemies(void);
extern void initMissionInfo(void);
extern char *getTranslatedString(char *string);

extern Battle battle;
extern Entity *player;
extern Game game;
