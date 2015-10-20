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

#define SHOW_BATTLE			0
#define SHOW_MENU			1
#define SHOW_OBJECTIVES		2
#define SHOW_OPTIONS		3

extern void prepareScene(void);
extern void presentScene(void);
extern void doBullets(void);
extern void drawBullets(void);
extern void doStars(float dx, float dy);
extern void drawStars(void);
extern void doFighters(void);
extern void drawFighters(void);
extern void initStars(void);
extern void doPlayer(void);
extern void drawHud(void);
extern void drawEffects(void);
extern void doEffects(void);
extern void doObjectives(void);
extern void blit(SDL_Texture *texture, int x, int y, int centered);
extern void initHud(void);
extern void initGalacticMap(void);
extern void drawWidgets(char *groupName);
extern void selectWidget(const char *name, const char *group);
extern Widget *getWidget(const char *name, const char *group);
extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
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

extern App app;
extern Battle battle;
extern Colors colors;
extern Fighter *player;
extern Game game;
