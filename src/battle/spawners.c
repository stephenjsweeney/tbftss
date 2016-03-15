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

#include "spawners.h"

void doSpawners(void)
{
	Entity *e;
	Spawner *s;
	char *type;
	int i, num;
	
	for (s = battle.spawnerHead.next ; s != NULL ; s = s->next)
	{
		if (s->active && --s->time <= 0)
		{
			num = s->step;
			
			if (s->limit)
			{
				num = MIN(s->step, s->total);
				
				s->total -= num;
			}
			
			for (i = 0 ; i < num ; i++)
			{
				type = s->types[rand() % s->numTypes];
				
				e = spawnFighter(type, 0, 0, s->side);
				
				if (s->offscreen)
				{
					e->x = player->x;
					e->y = player->y;
				}
				else
				{
					e->x = rand() % 2 ? 0 : BATTLE_AREA_WIDTH;
					e->y = rand() % 2 ? 0 : BATTLE_AREA_HEIGHT;
				}
				
				e->x += (rand() % 2) ? -SCREEN_WIDTH : SCREEN_WIDTH;
				e->y += (rand() % 2) ? -SCREEN_HEIGHT : SCREEN_HEIGHT;
				
				e->aiFlags |= AIF_UNLIMITED_RANGE;
			}
			
			s->time = s->interval;
		}
	}
}
