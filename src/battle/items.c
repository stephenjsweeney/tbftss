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
		
		STRNCPY(e->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(e->defName, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		e->texture = getTexture(cJSON_GetObjectItem(node, "textureName")->valuestring);
		
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
			e->alive = ALIVE_DEAD;
			printf("Picked up an Item\n");
		}
	}
}

void destroyItemDefs(void)
{
	Entity *f;
	
	while (defHead.next)
	{
		f = defHead.next;
		defHead.next = f->next;
		free(f);
	}
}
