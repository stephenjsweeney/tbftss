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

#include "extractionPoint.h"

static void think(void);
static void handleFleeingEntities(void);

Entity *spawnExtractionPoint(void)
{
	Entity *extractionPoint = spawnEntity();
	
	extractionPoint->type = ET_EXTRACTION_POINT;
	extractionPoint->health = extractionPoint->maxHealth = FPS;
	extractionPoint->texture = getTexture("gfx/entities/extractionPoint.png");
	extractionPoint->action = think;
	extractionPoint->flags |= EF_NO_MT_BOX;
	
	return extractionPoint;
}

static void think(void)
{
	self->thinkTime = 4;
	
	self->angle++;
	if (self->angle >= 360)
	{
		self->angle -= 360;
	}
	
	handleFleeingEntities();
	
	battle.extractionPoint = self;
}

static void handleFleeingEntities(void)
{
	Entity *e, **candidates;
	int i;
	
	candidates = getAllEntsWithin(self->x, self->y, self->w, self->h, self);
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->health > 0 && e->flags & EF_FLEEING && getDistance(e->x, e->y, self->x, self->y) <= 64)
		{
			e->alive = ALIVE_ESCAPED;
		}
	}
}
