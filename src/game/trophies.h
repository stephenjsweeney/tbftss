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

#include "time.h"

#define TROPHIES_PER_PAGE			4

extern void awardPandoranCraftTrophy(void);
extern void blit(AtlasImage *atlasImage, int x, int y, int centered);
extern void blitRotated(AtlasImage *atlasImage, int x, int y, float angle);
extern void blitScaled(AtlasImage *atlasImage, int x, int y, int w, int h, int center);
extern void calcTextDimensions(char *text, int size, int *w, int *h);
extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
extern void drawWidgets(char *groupName);
extern AtlasImage *getAtlasImage(char *filename);
extern int getJSONValue(cJSON *node, char *name, int defValue);
extern int getPercent(float current, float total);
extern Widget *getWidget(const char *name, const char *group);
extern long lookup(char *name);
extern float mod(float n, float x);
extern void playSound(int id);
extern char *readFile(char *filename);
extern void setAtlasColor(int r, int g, int b, int a);
extern char *timeToDate(long millis);

extern App app;
extern Battle battle;
extern Colors colors;
extern Entity *player;
extern Game game;
