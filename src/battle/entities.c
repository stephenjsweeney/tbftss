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

#include "entities.h"

static void drawEntity(Entity *e);
static void doEntity(Entity *prev);

Entity *spawnEntity(void)
{
	Entity *e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	e->id = battle.entId++;
	e->active = 1;
	
	battle.entityTail->next = e;
	battle.entityTail = e;
	
	return e;
}

void doEntities(void)
{
	Entity *e, *prev;
	
	prev = &battle.entityHead;
	
	battle.numAllies = battle.numEnemies = 0;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (!e->active)
		{
			continue;
		}
		
		self = e;
		
		if (self->target != NULL && self->target->health <= 0)
		{
			self->action = self->defaultAction;
			self->target = NULL;
		}
		
		self->x += self->dx;
		self->y += self->dy;
		
		if (self != player)
		{
			self->x -= battle.ssx;
			self->y -= battle.ssy;
		}
		
		if (self->action != NULL)
		{
			if (--self->thinkTime <= 0)
			{
				self->thinkTime = 0;
				self->action();
			}
		}
		
		switch (self->type)
		{
			case ET_FIGHTER:
				doFighter(prev);
				break;
				
			default:
				doEntity(prev);
				break;
		}
		
		prev = self;
	}
}

static void doEntity(Entity *prev)
{
	if (self->health <= 0)
	{
		if (self == battle.entityTail)
		{
			battle.entityTail = prev;
		}
		
		if (self == battle.missionTarget)
		{
			battle.missionTarget = NULL;
		}
		
		prev->next = self->next;
		free(self);
		self = prev;
	}
}

void drawEntities(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (!e->active)
		{
			continue;
		}
		
		switch (e->type)
		{
			case ET_FIGHTER:
				drawFighter(e);
				break;
				
			default:
				drawEntity(e);
				break;
		}
	}
}

static void drawEntity(Entity *e)
{
	blitRotated(e->texture, e->x, e->y, e->angle);
}
