/*
Copyright (C) 2018-2019 Parallel Realities

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

#include "stdlib.h"
#include "string.h"

#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "../common.h"

#define FONT_SIZE			32
#define FONT_TEXTURE_SIZE	512
#define MAX_WORD_LENGTH		128

extern char *getFileLocation(char *filename);
extern unsigned long hashcode(const char *str);
extern char *readFile(char *filename);
extern SDL_Texture *toTexture(SDL_Surface *surface, int destroySurface);

extern App app;
