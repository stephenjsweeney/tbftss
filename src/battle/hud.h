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

#define MAX_HUD_MESSAGES	6

extern SDL_Texture *getTexture(char *filename);
extern void blitRotated(SDL_Texture *texture, int x, int y, float angle);
extern void blit(SDL_Texture *texture, int x, int y, int center);
extern float getAngle(int x1, int y1, int x2, int y2);
extern void drawText(int x, int y, int size, int align, SDL_Color c, const char *format, ...);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void drawRadar(void);
extern void drawRadarRangeWarning(void);
extern int getPercent(float current, float total);
extern int playerHasGun(int type);
extern char *getTranslatedString(char *string);
extern char *timeToString(long millis, int showHours);
extern int jumpgateEnabled(void);

extern App app;
extern Battle battle;
extern Colors colors;
extern Dev dev;
extern Entity *player;
extern Game game;
