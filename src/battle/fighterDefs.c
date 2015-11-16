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

#include "fighterDefs.h"

static void loadFighterDef(char *filename);

static Entity defHead, *defTail;

Entity *getFighterDef(char *name)
{
	Entity *f;
	
	for (f = defHead.next ; f != NULL ; f = f->next)
	{
		if (strcmp(f->name, name) == 0)
		{
			return f;
		}
	}
	
	printf("Error: no such fighter '%s'\n", name);
	exit(1);
}

void loadFighterDefs(void)
{
	cJSON *root, *node;
	char *text;
	
	text = readFile("data/fighters/list.json");
	root = cJSON_Parse(text);
	
	memset(&defHead, 0, sizeof(Entity));
	defTail = &defHead;
	
	for (node = root->child ; node != NULL ; node = node->next)
	{
		loadFighterDef(node->valuestring);
	}
	
	cJSON_Delete(root);
	free(text);
}

static void loadFighterDef(char *filename)
{
	cJSON *root, *node;
	char *text;
	Entity *f;
	int i;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);
	
	text = readFile(filename);
	
	f = malloc(sizeof(Entity));
	memset(f, 0, sizeof(Entity));
	defTail->next = f;
	defTail = f;
	
	f->type = ET_FIGHTER;
	f->active = 1;
	
	root = cJSON_Parse(text);
	
	STRNCPY(f->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
	STRNCPY(f->defName, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
	f->health = f->maxHealth = cJSON_GetObjectItem(root, "health")->valueint;
	f->shield = f->maxShield = cJSON_GetObjectItem(root, "shield")->valueint;
	f->speed = cJSON_GetObjectItem(root, "speed")->valuedouble;
	f->reloadTime = cJSON_GetObjectItem(root, "reloadTime")->valueint;
	f->shieldRechargeRate = cJSON_GetObjectItem(root, "shieldRechargeRate")->valueint;
	f->texture = getTexture(cJSON_GetObjectItem(root, "textureName")->valuestring);
	
	SDL_QueryTexture(f->texture, NULL, NULL, &f->w, &f->h);
	
	if (cJSON_GetObjectItem(root, "guns"))
	{
		i = 0;
		
		for (node = cJSON_GetObjectItem(root, "guns")->child ; node != NULL ; node = node->next)
		{
			f->guns[i].type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			f->guns[i].x = cJSON_GetObjectItem(node, "x")->valueint;
			f->guns[i].y = cJSON_GetObjectItem(node, "y")->valueint;
			
			i++;
			
			if (i >= MAX_FIGHTER_GUNS)
			{
				printf("ERROR: cannot assign more than %d guns to a fighter\n", MAX_FIGHTER_GUNS);
				exit(1);
			}
		}
		
		if (cJSON_GetObjectItem(root, "combinedGuns"))
		{
			f->combinedGuns = cJSON_GetObjectItem(root, "combinedGuns")->valueint;
		}
	}
	
	f->selectedGunType = f->guns[0].type;
	
	if (cJSON_GetObjectItem(root, "missiles"))
	{
		node = cJSON_GetObjectItem(root, "missiles");
		
		f->missiles.type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
		f->missiles.ammo = f->missiles.maxAmmo = cJSON_GetObjectItem(node, "ammo")->valueint;
	}
	
	if (cJSON_GetObjectItem(root, "flags"))
	{
		f->flags = flagsToLong(cJSON_GetObjectItem(root, "flags")->valuestring);
	}
	
	f->separationRadius = MAX(f->w, f->h);
	f->separationRadius *= 2;
	
	/* all craft default to 100 system power */
	f->systemPower = 100;
	
	cJSON_Delete(root);
	free(text);
}

void destroyFighterDefs(void)
{
	Entity *f;
	
	while (defHead.next)
	{
		f = defHead.next;
		defHead.next = f->next;
		free(f);
	}
}
