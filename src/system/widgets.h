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

extern void blit(AtlasImage *atlasImage, int x, int y, int centered);
extern void calcTextDimensions(char *text, int size, int *w, int *h);
extern void clearControlConfig(char *name);
extern void clearInput(void);
extern int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
extern AtlasImage *getAtlasImage(char *filename);
extern char **getFileList(char *dir, int *count);
extern long lookup(char *name);
extern void playSound(int id);
extern char *readFile(char *filename);
extern void updateControlButton(char *name);
extern void updateControlKey(char *name);
extern void setAtlasColor(int r, int g, int b, int a);

extern App app;
extern Colors colors;
