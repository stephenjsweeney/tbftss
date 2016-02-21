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
	
	text = readFile(getFileLocation("data/battle/items.json"));
	
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
		STRNCPY(e->defName, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		e->texture = getTexture(cJSON_GetObjectItem(node, "texture")->valuestring);
		
		e->health = e->maxHealth = FPS;
		
		SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
		
		defTail->next = e;
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
	
	item->dx = rand() % 100 - rand() % 100;
	item->dx *= 0.01;
	item->dy = rand() % 100 - rand() % 100;
	item->dy *= 0.01;
	item->action = action;
	
	return item;
}

static Entity *getItemDef(char *name)
{
	Entity *e;
	
	for (e = defHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, name) == 0)
		{
			return e;
		}
	}
	
	printf("Error: no such item '%s'\n", name);
	exit(1);
}

static void action(void)
{
	Entity *e, **candidates;
	int i;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if ((e->flags & EF_COLLECTS_ITEMS) && collision(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, e->x - (e->w / 2), e->y - (e->h / 2), e->w, e->h))
		{
			self->health = 0;
			playBattleSound(SND_GET_ITEM, self->x, self->y);
			
			updateObjective(self->name, TT_ITEM);
			
			if (e == player)
			{
				addHudMessage(colors.white, "Picked up %s", self->name);
				battle.stats[STAT_ITEMS_COLLECTED]++;
			}
			
			self->action = NULL;
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
