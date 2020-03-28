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

#include "SDL2/SDL_mixer.h"

#define SHOW_CONTROLS    1
#define SHOW_MAIN        0

extern void drawControls(void);
extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
extern void drawWidgets(char *groupName);
extern Widget *getWidget(const char *name, const char *group);
extern void initControlsDisplay(void);
extern void initGraphics(void);
extern void initStars(void);
extern void saveConfig(void);
extern void selectWidget(const char *name, const char *group);
extern void setWidgetOption(const char *name, const char *group, const char *value);

extern App app;
extern Colors colors;
