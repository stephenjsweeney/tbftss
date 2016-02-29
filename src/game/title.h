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

#define SHOW_TITLE				0
#define SHOW_STATS				1
#define SHOW_OPTIONS			2

#define NUM_FIGHTERS 12

extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
extern SDL_Texture *getTexture(const char *filename);
extern void startSectionTransition(void);
extern void endSectionTransition(void);
extern void blit(SDL_Texture *t, int x, int y, int centered);
extern void stopMusic(void);
extern void doStars(float dx, float dy);
extern void drawStars(void);
extern void drawBackground(SDL_Texture *texture);
extern void initBackground(void);
extern void scrollBackground(float x, float y);
extern void addEngineEffect(void);
extern void doEffects(void);
extern void drawEffects(void);
extern void drawWidgets(char *groupName);
extern void doWidgets(void);
extern Widget *getWidget(const char *name, const char *group);
extern void initGalacticMap(void);
extern void initOptions(void (*returnFromOptions)(void));
extern void drawOptions(void);
extern void initStatsDisplay(void);
extern void playMusic(char *filename);
extern void destroyBattle(void);
extern void playSound(int id);
extern void initEffects(void);
extern void initChallengeHome(void);
extern void selectWidget(const char *name, const char *group);
extern void drawStats(void);
extern void updateAllMissions(void);

extern App app;
extern Battle battle;
extern Colors colors;
extern Entity *self;
