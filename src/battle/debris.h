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

#define INITIAL_DEBRIS_DRAW_CAPACITY    32
#define MAX_DEBRIS_TEXTURES             6

extern void addDebrisFire(int x, int y);
extern void blitRotated(AtlasImage *atlasImage, int x, int y, float angle);
extern AtlasImage *getAtlasImage(char *filename);
extern int isOnBattleScreen(int x, int y, int w, int h);
extern float mod(float n, float x);
extern void *resize(void *array, int oldSize, int newSize);

extern Battle battle;
