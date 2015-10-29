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
static void doEntity(void);

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
		self = e;
		
		if (!e->active)
		{
			continue;
		}
		
		if (e->target != NULL && e->target->health <= 0)
		{
			e->action = e->defaultAction;
			e->target = NULL;
		}
		
		e->x += e->dx;
		e->y += e->dy;
		
		if (e != player)
		{
			e->x -= battle.ssx;
			e->y -= battle.ssy;
		}
		
		if (e->action != NULL)
		{
			if (--e->thinkTime <= 0)
			{
				e->thinkTime = 0;
				e->action();
			}
		}
		
		switch (e->type)
		{
			case ET_FIGHTER:
				doFighter();
				break;
				
			default:
				doEntity();
				break;
		}
		
		if (e->alive == ALIVE_DEAD)
		{
			if (e == battle.entityTail)
			{
				battle.entityTail = prev;
			}
			
			if (e == battle.missionTarget)
			{
				battle.missionTarget = NULL;
			}
			
			if (e == player)
			{
				player = NULL;
			}
			
			prev->next = e->next;
			free(e);
			e = prev;
		}
		
		prev = e;
	}
}

static void doEntity(void)
{
	if (self->health <= 0)
	{
		self->alive = ALIVE_DEAD;
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
