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

void doEntities(void)
{
	Entity *e, *prev;
	
	prev = &battle.entityHead;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		e->x += e->dx;
		e->y += e->dy;
		
		e->x -= battle.ssx;
		e->y -= battle.ssy;
		
		if (e->action != NULL)
		{
			if (--e->thinkTime <= 0)
			{
				e->action();
			}
		}
		
		if (e->health <= 0)
		{
			prev->next = e->next;
			free(e);
			e = prev;
		}
		
		prev = e;
	}
}

void drawEntities(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		blitRotated(e->texture, e->x, e->y, e->angle);
	}
}
