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
	int i, num, addFlags, addAIFlags;
	long flags, aiFlags;
	
	for (s = battle.spawnerHead.next ; s != NULL ; s = s->next)
	{
		if (s->active && --s->time <= 0)
		{
			aiFlags = flags = -1;
			
			num = s->step;
			
			if (s->total != -1)
			{
				num = MIN(s->step, s->total);
				
				s->total -= num;
			}
			
			if (s->side != SIDE_ALLIES)
			{
				battle.numInitialEnemies += num;
			}
			
			if (strlen(s->flags))
			{
				flags = flagsToLong(s->flags, &addFlags);
			}

			if (strlen(s->aiFlags))
			{
				aiFlags = flagsToLong(s->aiFlags, &addAIFlags);
			}
			
			for (i = 0 ; i < num ; i++)
			{
				type = s->types[rand() % s->numTypes];
				
				e = spawnFighter(type, 0, 0, s->side);
				
				e->spawned = 1;
				
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
				
				if (flags != -1)
				{
					if (addFlags)
					{
						e->flags |= flags;
					}
					else
					{
						e->flags = flags;
					}
				}
				
				if (aiFlags != -1)
				{
					if (addAIFlags)
					{
						e->aiFlags |= aiFlags;
					}
					else
					{
						e->aiFlags = aiFlags;
					}
				}
			}
			
			s->time = s->interval;
		}
	}
}

void activateSpawner(char *name, int active)
{
	Spawner *s;

	for (s = battle.spawnerHead.next ; s != NULL ; s = s->next)
	{
		if (strcmp(s->name, name) == 0)
		{
			s->active = active;
		}
	}
}

void activateTrespasserSpawner(void)
{
	Spawner *s;
	char types[MAX_DESCRIPTION_LENGTH];
	
	s = malloc(sizeof(Spawner));
	memset(s, 0, sizeof(Spawner));
	battle.spawnerTail->next = s;
	battle.spawnerTail = s;
	
	STRNCPY(types, "Jackal;Mantis;Sphinx;Scarab", MAX_DESCRIPTION_LENGTH);

	s->types = toTypeArray(types, &s->numTypes);
	s->side = SIDE_PANDORAN;
	s->interval = 5 * FPS;
	s->total = -1;
	s->step = 5;
	s->offscreen = 1;
	s->active = 1;
	STRNCPY(s->flags, "+EF_IMMORTAL", MAX_DESCRIPTION_LENGTH);
	STRNCPY(s->aiFlags, "+AIF_UNLIMITED_RANGE", MAX_DESCRIPTION_LENGTH);
}

void loadSpawners(cJSON *node)
{
	Spawner *s;

	if (node)
	{
		node = node->child;

		while (node)
		{
			s = malloc(sizeof(Spawner));
			memset(s, 0, sizeof(Spawner));
			battle.spawnerTail->next = s;
			battle.spawnerTail = s;

			STRNCPY(s->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			s->types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &s->numTypes);
			s->side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			s->interval = cJSON_GetObjectItem(node, "interval")->valueint * FPS;
			s->total = getJSONValue(node, "total", 0);
			s->step = cJSON_GetObjectItem(node, "step")->valueint;
			s->offscreen = getJSONValue(node, "offscreen", 0);
			s->active = getJSONValue(node, "active", 1);
			STRNCPY(s->flags, getJSONValueStr(node, "flags", ""), MAX_DESCRIPTION_LENGTH);
			STRNCPY(s->aiFlags, getJSONValueStr(node, "aiFlags", ""), MAX_DESCRIPTION_LENGTH);

			node = node->next;
		}
	}
}
