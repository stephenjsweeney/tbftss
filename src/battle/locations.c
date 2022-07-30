/*
Copyright (C) 2015-2019,2022 Parallel Realities

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
#include "locations.h"
#include "../json/cJSON.h"
#include "../system/draw.h"
#include "../system/util.h"
#include "../battle/script.h"

extern Battle battle;
extern Entity *player;

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

/*
 * Literally only used when Christabel's shuttle is disabled
 */
void createChristabelLocation(void)
{
	Location *l;
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, "Christabel") == 0)
		{
			l = malloc(sizeof(Location));
			memset(l, 0, sizeof(Location));
			battle.locationTail->next = l;
			battle.locationTail = l;

			STRNCPY(l->name, "CristabelLocation", MAX_NAME_LENGTH);
			l->x = e->x;
			l->y = e->y;
			l->size = 500;
			l->active = 1;

			l->x -= l->size / 2;
			l->y -= l->size / 2;

			return;
		}
	}
}

void loadLocations(cJSON *node)
{
	int active;
	Location *l;

	if (node)
	{
		node = node->child;

		while (node)
		{
			l = malloc(sizeof(Location));
			memset(l, 0, sizeof(Location));
			battle.locationTail->next = l;
			battle.locationTail = l;

			STRNCPY(l->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			l->x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			l->y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			l->size = cJSON_GetObjectItem(node, "size")->valueint;
			l->active = active = getJSONValue(node, "active", 1);

			l->x += (SCREEN_WIDTH / 2);
			l->y += (SCREEN_HEIGHT / 2);

			node = node->next;
		}
	}
}
