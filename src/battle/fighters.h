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

extern SDL_Texture *getTexture(char *filename);
extern void doAI(void);
extern void blitRotated(SDL_Texture *t, int x, int y, int angle);
extern void blit(SDL_Texture *t, int x, int y, int center);
extern float getAngle(int x1, int y1, int x2, int y2);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void addEngineEffect(void);
extern void addFighterExplosion(void);
extern void addSmallFighterExplosion(void);
extern void playBattleSound(int id, int x, int y);
extern void updateObjective(char *name);
extern void updateCondition(char *name);
extern Fighter *getFighterDef(char *name);
extern void addHudMessage(SDL_Color c, char *format, ...);

extern App app;
extern Battle battle;
extern Colors colors;
extern Fighter *player;
extern Fighter *self;
