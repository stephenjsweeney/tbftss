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

#include "locations.h"

void doLocations(void)
{
	Location *l, *prev;
	
	prev = &battle.locationHead;
	
	for (l = battle.locationHead.next ; l != NULL ; l = l->next)
	{
		if (l->active && getDistance(player->x, player->y, l->x, l->y) <= l->size)
		{
			runScriptFunction(l->name);
			
			prev->next = l->next;
			free(l);
			l = prev;
		}
		
		prev = l;
	}
}

void drawLocations(void)
{
	Location *l;
	
	for (l = battle.locationHead.next ; l != NULL ; l = l->next)
	{
		if (l->active)
		{
			drawCircle(l->x - battle.camera.x, l->y - battle.camera.y, l->size, 0, 255, 0, 255);
		}
	}
}

void activateLocations(char *locations)
{
	char *token;
	Location *l;
	
	token = strtok(locations, ";");
	
	while (token)
	{
		for (l = battle.locationHead.next ; l != NULL ; l = l->next)
		{
			if (strcmp(token, l->name) == 0)
			{
				l->active = 1;
			}
		}
		
		token = strtok(NULL, ";");
	}
}
