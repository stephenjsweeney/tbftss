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

#define SHOW_BATTLE			0
#define SHOW_MENU			1
#define SHOW_OBJECTIVES		2
#define SHOW_OPTIONS		3

extern void awardTrophy(char *id);
extern void blitScaled(AtlasImage *atlasImage, int x, int y, int w, int h, int center);
extern void cancelScript(void);
extern void clearInput(void);
extern void destroyBullets(void);
extern void destroyDebris(void);
extern void destroyEffects(void);
extern void destroyEntities(void);
extern void destroyQuadtree(void);
extern void destroyScript(void);
extern void doBullets(void);
extern void doChallenges(void);
extern void doDebris(void);
extern void doEffects(void);
extern void doEntities(void);
extern void doHud(void);
extern void doLocations(void);
extern void doMessageBox(void);
extern void doObjectives(void);
extern void doPlayer(void);
extern void doPlayerSelect(void);
extern void doScript(void);
extern void doSpawners(void);
extern void doStars(float dx, float dy);
extern void doWidgets(void);
extern void drawBackground(SDL_Texture *texture);
extern void drawBullets(void);
extern void drawDebris(void);
extern void drawEffects(void);
extern void drawEntities(void);
extern void drawHud(void);
extern void drawLocations(void);
extern void drawMessageBox(void);
extern void drawMissionInfo(void);
extern void drawOptions(void);
extern void drawStars(void);
extern void drawWidgets(char *groupName);
extern Widget *getWidget(const char *name, const char *group);
extern void initBullets(void);
extern void initChallengeHome(void);
extern void initCredits(void);
extern void initDebris(void);
extern void initEffects(void);
extern void initEntities(void);
extern void initGalacticMap(void);
extern void initHud(void);
extern void initMessageBox(void);
extern void initOptions(void (*returnFromOptions)(void));
extern void initQuadtree(Quadtree *root);
extern void initRadar(void);
extern void initStars(void);
extern void loadMission(char *filename);
extern void playSound(int id);
extern void resetHud(void);
extern void resetMessageBox(void);
extern void resetWaypoints(void);
extern void runScriptFunction(const char *format, ...);
extern void scrollBackground(float x, float y);
extern void selectWidget(const char *name, const char *group);
extern void setAtlasColor(int r, int g, int b, int a);
extern void showOKCancelDialog(void (*okCallback)(void), void (*cancelCallback)(void), const char *format, ...);
extern void updateAccuracyStats(unsigned int *stats);

extern App app;
extern Battle battle;
extern Dev dev;
extern Entity *player;
extern Game game;
