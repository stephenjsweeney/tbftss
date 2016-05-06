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

#include "items.h"

static void action(void);
static Entity *getItemDef(char *name);

static Entity defHead, *defTail;

void loadItemDefs(void)
{
	cJSON *root, *node;
	char *text;
	Entity *e;

	text = readFile("data/battle/items.json");

	root = cJSON_Parse(text);

	memset(&defHead, 0, sizeof(Entity));
	defTail = &defHead;

	for (node = root->child ; node != NULL ; node = node->next)
	{
		e = malloc(sizeof(Entity));
		memset(e, 0, sizeof(Entity));

		e->type = ET_ITEM;
		e->active = 1;
		STRNCPY(e->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(e->defName, cJSON_GetObjectItem(node, "defName")->valuestring, MAX_NAME_LENGTH);
		e->texture = getTexture(cJSON_GetObjectItem(node, "texture")->valuestring);
		e->health = e->maxHealth = FPS;
		e->flags = EF_NO_HEALTH_BAR;
		
		SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

		defTail->next = e;
		defTail = e;
	}

	cJSON_Delete(root);
	free(text);
}

Entity *spawnItem(char *name)
{
	Entity *item, *def;

	item = spawnEntity();

	def = getItemDef(name);

	memcpy(item, def, sizeof(Entity));
	
	item->action = action;

	return item;
}

void addRandomItem(int x, int y)
{
	Entity *e;

	e = spawnItem("smallCrate");
	e->x = x;
	e->y = y;
	
	e->speed = 1;
	e->dx = rand() % 200 - rand() % 200;
	e->dy = rand() % 200 - rand() % 200;
	e->dx *= 0.01;
	e->dy *= 0.01;
}

static Entity *getItemDef(char *defName)
{
	Entity *e;

	for (e = defHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->defName, defName) == 0)
		{
			return e;
		}
	}

	printf("Error: no such item '%s'\n", defName);
	exit(1);
}

static void action(void)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsInRadius(self->x, self->y, MAX(self->w, self->h), self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->alive == ALIVE_ALIVE && (e->flags & EF_COLLECTS_ITEMS) && collision(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, e->x - (e->w / 2), e->y - (e->h / 2), e->w, e->h))
		{
			self->health = 0;
			playBattleSound(SND_GET_ITEM, self->x, self->y);

			updateObjective(self->name, TT_ITEM);

			if (e == player)
			{
				addHudMessage(colors.white, _("Picked up %s"), self->name);
				battle.stats[STAT_ITEMS_COLLECTED_PLAYER]++;
			}
			else
			{
				battle.stats[STAT_ITEMS_COLLECTED]++;
			}

			self->action = NULL;
		}
	}
}

void loadItems(cJSON *node)
{
	Entity *e;
	char *name, *groupName, *type;
	int i, scatter, number, active, addFlags;
	long flags;
	float x, y;

	flags = -1;
	scatter = 1;

	if (node)
	{
		node = node->child;

		while (node)
		{
			type = cJSON_GetObjectItem(node, "type")->valuestring;
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			name = NULL;
			groupName = NULL;

			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			scatter = getJSONValue(node, "scatter", 1);
			active = getJSONValue(node, "active", 1);

			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				e = spawnItem(type);

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}

				if (flags != -1)
				{
					if (addFlags)
					{
						e->flags |= flags;
					}
					else
					{
						e->flags = flags;

						SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Flags for '%s' (%s) replaced", e->name, e->defName);
					}
				}

				e->x = x;
				e->y = y;
				e->active = active;

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			}

			node = node->next;
		}
	}
}

void destroyItemDefs(void)
{
	Entity *e;

	while (defHead.next)
	{
		e = defHead.next;
		defHead.next = e->next;
		free(e);
	}
}
