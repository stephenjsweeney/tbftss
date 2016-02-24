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

#include "../json/cJSON.h"

#define SHOW_BATTLE			0
#define SHOW_MENU			1
#define SHOW_OBJECTIVES		2
#define SHOW_OPTIONS		3

extern void doBullets(void);
extern void drawBullets(void);
extern void doStars(float dx, float dy);
extern void drawStars(void);
extern void doEntities(void);
extern void drawEntities(void);
extern void initStars(void);
extern void doPlayer(void);
extern void drawHud(void);
extern void drawEffects(void);
extern void doEffects(void);
extern void doObjectives(void);
extern void blitScaled(SDL_Texture *texture, int x, int y, int w, int h);
extern void initHud(void);
extern void initRadar(void);
extern void initGalacticMap(void);
extern void initEntities(void);
extern void destroyEntities(void);
extern void drawWidgets(char *groupName);
extern void selectWidget(const char *name, const char *group);
extern Widget *getWidget(const char *name, const char *group);
extern void doWidgets(void);
extern void loadMission(char *filename);
extern void resetHud(void);
extern void doHud(void);
extern void initMissionInfo(void);
extern void drawMissionInfo(void);
extern void drawBackground(SDL_Texture *texture);
extern void initBackground(void);
extern void scrollBackground(float x, float y);
extern void initOptions(void (*returnFromOptions)(void));
extern void drawOptions(void);
extern void playSound(int id);
extern void resetWaypoints(void);
extern void doPlayerSelect(void);
extern void destroyQuadtree(void);
extern void initQuadtree(Quadtree *root);
extern void initEffects(void);
extern void doScript(void);
extern void destroyScript(void);
extern void runScriptFunction(char *format, ...);
extern void initMessageBox(void);
extern void doMessageBox(void);
extern void drawMessageBox(void);
extern void resetMessageBox(void);
extern void initBullets(void);
extern void initDebris(void);
extern void doDebris(void);
extern void drawDebris(void);
extern void doLocations(void);
extern void drawLocations(void);
extern void destroyDebris(void);
extern void destroyBullets(void);
extern void destroyEffects(void);

extern App app;
extern Battle battle;
extern Dev dev;
extern Entity *player;
extern Game game;
