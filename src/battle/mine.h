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

#define TRIGGER_RANGE	150
#define DAMAGE_RANGE	255
#define SYSTEM_POWER	75

extern Entity *spawnEntity(void);
extern SDL_Texture *getTexture(char *filename);
extern Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore);
extern int getDistance(int x1, int y1, int x2, int y2);
extern void addMineExplosion(void);
extern void damageFighter(Entity *e, int amount, long flags);
extern void playBattleSound(int id, int x, int y);

extern Entity *self;
