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

#include "mine.h"

static void think(void);
static void lookForFighters(void);

Entity *spawnMine(void)
{
	Entity *mine = spawnEntity();

	mine->type = ET_MINE;
	mine->health = mine->maxHealth = 1;
	mine->texture = getTexture("gfx/entities/mine.png");
	mine->action = think;

	return mine;
}

static void think(void)
{
	self->angle += 0.1;
	
	if (self->angle >= 360)
	{
		self->angle -= 360;
	}
	
	lookForFighters();
}

static void lookForFighters(void)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->health > 0 && e->type == ET_FIGHTER && getDistance(self->x, self->y, e->x, e->y) <= 128)
		{
			self->health = 0;
			
			addMineExplosion();
		}
	}
}
