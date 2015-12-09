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

#include "capitalShips.h"

static void think(void);
static void gunThink(void);
static void componentDie(void);
static void loadCapitalShipDef(char *filename);
static void loadComponents(Entity *parent, cJSON *components);
static void loadGuns(Entity *parent, cJSON *guns);

static Entity defHead, *defTail;

Entity *spawnCapitalShip(char *name, int x, int y, int side)
{
	Entity *def, *e, *capitalShip;
	
	for (def = defHead.next ; def != NULL ; def = def->next)
	{
		if ((strcmp(def->name, name) == 0) || (def->owner != NULL && strcmp(def->owner->name, name) == 0))
		{
			e = spawnEntity();
	
			memcpy(e, def, sizeof(Entity));
			
			e->id = battle.entId;
			e->next = NULL;
			
			e->x = x;
			e->y = y;
			e->side = side;
			
			if (e->type == ET_CAPITAL_SHIP)
			{
				capitalShip = e;
			}
			else
			{
				e->owner = capitalShip;
			}
		}
	}
	
	return capitalShip;
}

static void think(void)
{
	
}

static void gunThink(void)
{
	/*doAI();*/
}

static void componentDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	
	if (self->type == ET_CAPITAL_SHIP_COMPONENT)
	{
		self->owner->health--;
	}
}

static void die(void)
{
	self->alive = ALIVE_DEAD;
}

void loadCapitalShipDefs(void)
{
	cJSON *root, *node;
	char *text;
	
	text = readFile(getFileLocation("data/capitalShips/list.json"));
	root = cJSON_Parse(text);
	
	memset(&defHead, 0, sizeof(Entity));
	defTail = &defHead;
	
	for (node = root->child ; node != NULL ; node = node->next)
	{
		loadCapitalShipDef(node->valuestring);
	}
	
	cJSON_Delete(root);
	free(text);
}

static void loadCapitalShipDef(char *filename)
{
	cJSON *root;
	char *text;
	Entity *e;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);
	
	text = readFile(getFileLocation(filename));
	
	e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	defTail->next = e;
	defTail = e;
	
	e->type = ET_CAPITAL_SHIP;
	e->active = 1;
	
	root = cJSON_Parse(text);
	
	STRNCPY(e->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
	STRNCPY(e->defName, e->name, MAX_NAME_LENGTH);
	e->health = e->maxHealth = cJSON_GetObjectItem(root, "health")->valueint;
	e->shield = e->maxShield = cJSON_GetObjectItem(root, "shield")->valueint;
	e->shieldRechargeRate = cJSON_GetObjectItem(root, "shieldRechargeRate")->valueint;
	e->texture = getTexture(cJSON_GetObjectItem(root, "texture")->valuestring);
	
	e->action = think;
	e->die = die;
	
	SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
	
	loadComponents(e, cJSON_GetObjectItem(root, "components"));
	
	loadGuns(e, cJSON_GetObjectItem(root, "guns"));
	
	cJSON_Delete(root);
	free(text);
}

static void loadComponents(Entity *parent, cJSON *components)
{
	Entity *e;
	cJSON *component;
	
	parent->health = 0;
	
	if (components)
	{
		component = components->child;
		
		while (component)
		{
			e = malloc(sizeof(Entity));
			memset(e, 0, sizeof(Entity));
			defTail->next = e;
			defTail = e;
			
			e->active = 1;
	
			e->type = ET_CAPITAL_SHIP_COMPONENT;
			sprintf(e->name, "%s (Component)", parent->name);
			sprintf(e->defName, "%s (Component)", parent->defName);
			e->health = e->maxHealth = cJSON_GetObjectItem(component, "health")->valueint;
			e->offsetX = cJSON_GetObjectItem(component, "x")->valueint;
			e->offsetY = cJSON_GetObjectItem(component, "y")->valueint;
			e->texture = getTexture(cJSON_GetObjectItem(component, "texture")->valuestring);
			
			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			
			if (cJSON_GetObjectItem(component, "aiFlags"))
			{
				e->aiFlags = flagsToLong(cJSON_GetObjectItem(component, "aiFlags")->valuestring);
			}
			
			if (cJSON_GetObjectItem(component, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(component, "flags")->valuestring);
			}
			
			e->systemPower = 100;
			
			e->die = componentDie;
			
			e->owner = parent;
			
			component = component->next;
			
			parent->health++;
		}
	}
}

static void loadGuns(Entity *parent, cJSON *guns)
{
	Entity *e;
	cJSON *gun;
	
	if (guns)
	{
		gun = guns->child;
		
		while (gun)
		{
			e = malloc(sizeof(Entity));
			memset(e, 0, sizeof(Entity));
			defTail->next = e;
			defTail = e;
			
			e->active = 1;
	
			e->type = ET_CAPITAL_SHIP_GUN;
			sprintf(e->name, "%s (Cannon)", parent->name);
			sprintf(e->defName, "%s (Cannon)", parent->defName);
			e->health = e->maxHealth = cJSON_GetObjectItem(gun, "health")->valueint;
			e->reloadTime = cJSON_GetObjectItem(gun, "reloadTime")->valueint;
			e->offsetX = cJSON_GetObjectItem(gun, "x")->valueint;
			e->offsetY = cJSON_GetObjectItem(gun, "y")->valueint;
			e->texture = getTexture(cJSON_GetObjectItem(gun, "texture")->valuestring);
			e->guns[0].type = lookup(cJSON_GetObjectItem(gun, "type")->valuestring);
			
			if (cJSON_GetObjectItem(gun, "aiFlags"))
			{
				e->aiFlags = flagsToLong(cJSON_GetObjectItem(gun, "aiFlags")->valuestring);
			}
			
			if (cJSON_GetObjectItem(gun, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(gun, "flags")->valuestring);
			}
			
			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			
			e->systemPower = 100;
			
			e->action = gunThink;
			e->die = componentDie;
			
			e->owner = parent;
			
			gun = gun->next;
		}
	}
}

void destroyCapitalShipDefs(void)
{
	Entity *e;
	
	while (defHead.next)
	{
		e = defHead.next;
		defHead.next = e->next;
		free(e);
	}
}
