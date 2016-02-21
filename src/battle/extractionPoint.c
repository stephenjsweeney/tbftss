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

#include "extractionPoint.h"

static void think(void);
static void handleFleeingEntities(void);
static void addEscapeEffect(Entity *ent);

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
	
	candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->health > 0 && e->flags & EF_RETREATING && getDistance(self->x, self->y, e->x, e->y) <= 255)
		{
			e->alive = ALIVE_ESCAPED;
			
			addEscapeEffect(e);
			
			playSound(SND_JUMP);
		}
	}
}

static void addEscapeEffect(Entity *ent)
{
	Effect *e;
	int i, n, speed;
	
	n = ent->w * ent->h;
	
	for (i = 0 ; i < n ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;
		
		speed = 50 + rand() % 50;
		
		e->type = EFFECT_POINT;
		e->x = ent->x + (rand() % ent->w) - (rand() % ent->w);
		e->y = ent->y + (rand() % ent->h) - (rand() % ent->w);
		
		e->dx = self->x - e->x;
		e->dx /= speed;
		
		e->dy = self->y - e->y;
		e->dy /= speed;
		
		e->r = e->g = e->b = e->a = (rand() % 255);
		e->health = speed;
	}
}
