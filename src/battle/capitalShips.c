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

#include "capitalShips.h"

static int steer(void);
static void think(void);
static void gunThink(void);
static void gunDie(void);
static void handleDisabled(void);
static void componentDie(void);
static void engineThink(void);
static void engineDie(void);
static void findAITarget(void);
static void loadCapitalShipDef(char *filename);
static void loadComponents(Entity *parent, cJSON *components);
static void loadGuns(Entity *parent, cJSON *guns);
static void loadEngines(Entity *parent, cJSON *engines);

static Entity defHead, *defTail;

Entity *spawnCapitalShip(char *name, int x, int y, int side)
{
	Entity *def, *e, *capitalShip;

	capitalShip = NULL;

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

	if (!capitalShip)
	{
		printf("Error: no such capital ship '%s'\n", name);
		exit(1);
	}

	return capitalShip;
}

void doCapitalShip(void)
{
	if (self->alive == ALIVE_ALIVE)
	{
		handleDisabled();
		
		if (self->health <= 0)
		{
			self->health = 0;
			self->alive = ALIVE_DYING;
			self->die();

			if (self == battle.missionTarget)
			{
				battle.missionTarget = NULL;
			}

			if (self->side == SIDE_ALLIES)
			{
				battle.stats[STAT_CAPITAL_SHIPS_LOST]++;

				runScriptFunction("CAPITAL_SHIPS_LOST %d", battle.stats[STAT_CAPITAL_SHIPS_LOST]);
			}
			else
			{
				battle.stats[STAT_CAPITAL_SHIPS_DESTROYED]++;

				runScriptFunction("CAPITAL_SHIPS_DESTROYED %d", battle.stats[STAT_CAPITAL_SHIPS_DESTROYED]);
			}
		}
	}
}

static void think(void)
{
	float dir;
	int wantedAngle;

	if (--self->aiActionTime <= 0)
	{
		findAITarget();
	}

	wantedAngle = steer();

	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;

		dir *= TURN_SPEED;

		self->angle += dir;

		self->angle = fmod(self->angle, 360);
	}

	applyFighterThrust();
}

static void findAITarget(void)
{
	Entity *e;
	unsigned int dist, closest;

	self->target = NULL;
	dist = closest = MAX_TARGET_RANGE;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->side != self->side && e->flags & EF_AI_TARGET)
		{
			dist = getDistance(self->x, self->y, e->x, e->y);

			if (!self->target || dist < closest)
			{
				self->target = e;
				closest = dist;
			}
		}
	}

	if (self->target)
	{
		self->targetLocation.x = self->target->x + (rand() % 1000 - rand() % 1000);
		self->targetLocation.y = self->target->y + (rand() % 1000 - rand() % 1000);

		self->aiActionTime = FPS * 15;
	}
	else
	{
		self->targetLocation.x = 500 + (rand() % (BATTLE_AREA_WIDTH - 1000));
		self->targetLocation.y = 500 + (rand() % (BATTLE_AREA_HEIGHT - 1000));

		self->aiActionTime = FPS * (30 + (rand() % 120));
	}
}

static int steer(void)
{
	int wantedAngle;
	int angle;
	int distance;
	float dx, dy, force;
	int count;
	Entity *e, **candidates;
	int i;

	dx = dy = 0;
	count = 0;
	force = 0;

	candidates = getAllEntsWithin(self->x - 1000, self->y - 1000, 2000, 2000, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_CAPITAL_SHIP)
		{
			distance = getDistance(e->x, e->y, self->x, self->y);

			if (distance > 0 && distance < self->separationRadius)
			{
				angle = getAngle(self->x, self->y, e->x, e->y);

				dx += sin(TO_RAIDANS(angle));
				dy += -cos(TO_RAIDANS(angle));
				force += (self->separationRadius - distance);

				count++;
			}
		}
	}

	if (count > 0)
	{
		dx /= count;
		dy /= count;

		dx *= force;
		dy *= force;

		self->dx -= (dx * 0.001);
		self->dy -= (dy * 0.001);

		self->targetLocation.x -= dx;
		self->targetLocation.y -= dy;

		self->aiActionTime = FPS * 10;
	}

	wantedAngle = getAngle(self->x, self->y, self->targetLocation.x, self->targetLocation.y);

	wantedAngle %= 360;

	return wantedAngle;
}

static void gunThink(void)
{
	handleDisabled();
	
	doAI();
}

static void componentDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	addDebris(self->x, self->y, 3 + rand() % 4);

	self->owner->health--;

	if (self->owner->health > 0)
	{
		runScriptFunction("CAP_HEALTH %s %d", self->owner->name, self->owner->health);
	}
}

static void gunDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	addDebris(self->x, self->y, 3 + rand() % 4);
}

static void engineThink(void)
{
	handleDisabled();
	
	addLargeEngineEffect();
}

static void engineDie(void)
{
	Entity *e;

	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	addDebris(self->x, self->y, 4 + rand() % 9);

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e != self && e->owner == self->owner && e->type == ET_CAPITAL_SHIP_ENGINE)
		{
			return;
		}
	}

	/* no more engines - stop moving */
	if (self->owner->health > 0)
	{
		self->owner->speed = 0;
		self->owner->action = NULL;
		self->owner->dx = self->owner->dy = 0;

		runScriptFunction("CAP_ENGINES_DESTROYED %s", self->owner->name);
	}
}

static void die(void)
{
	Entity *e;

	self->alive = ALIVE_DEAD;

	addLargeExplosion();
	
	addDebris(self->x, self->y, 12);

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->owner == self)
		{
			e->alive = ALIVE_DEAD;
		}
	}

	updateObjective(self->name, TT_DESTROY);

	updateObjective(self->groupName, TT_DESTROY);
}

static void handleDisabled(void)
{
	if (self->systemPower <= 0 || (self->flags & EF_DISABLED))
	{
		self->dx *= 0.99;
		self->dy *= 0.99;
		self->thrust = 0;
		self->shield = self->maxShield = 0;
		self->action = NULL;
	}
}

void loadCapitalShipDefs(void)
{
	char **filenames;
	char path[MAX_FILENAME_LENGTH];
	int count, i;

	memset(&defHead, 0, sizeof(Entity));
	defTail = &defHead;

	filenames = getFileList("data/capitalShips", &count);

	for (i = 0 ; i < count ; i++)
	{
		sprintf(path, "data/capitalShips/%s", filenames[i]);

		loadCapitalShipDef(path);

		free(filenames[i]);
	}

	free(filenames);
}

static void loadCapitalShipDef(char *filename)
{
	cJSON *root;
	char *text;
	Entity *e;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);

	root = cJSON_Parse(text);

	if (root)
	{
		e = malloc(sizeof(Entity));
		memset(e, 0, sizeof(Entity));
		defTail->next = e;
		defTail = e;

		e->type = ET_CAPITAL_SHIP;
		e->active = 1;

		STRNCPY(e->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(e->defName, e->name, MAX_NAME_LENGTH);
		e->shield = e->maxShield = cJSON_GetObjectItem(root, "shield")->valueint;
		e->shieldRechargeRate = cJSON_GetObjectItem(root, "shieldRechargeRate")->valueint;
		e->texture = getTexture(cJSON_GetObjectItem(root, "texture")->valuestring);
		e->speed = 1;
		e->systemPower = MAX_SYSTEM_POWER;

		e->action = think;
		e->die = die;

		SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

		e->separationRadius = MAX(e->w, e->h);

		loadComponents(e, cJSON_GetObjectItem(root, "components"));

		loadGuns(e, cJSON_GetObjectItem(root, "guns"));

		loadEngines(e, cJSON_GetObjectItem(root, "engines"));

		cJSON_Delete(root);
	}
	else
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Failed to load '%s'", filename);
	}

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
			e->health = e->maxHealth = cJSON_GetObjectItem(component, "health")->valueint;
			e->offsetX = cJSON_GetObjectItem(component, "x")->valueint;
			e->offsetY = cJSON_GetObjectItem(component, "y")->valueint;
			e->texture = getTexture(cJSON_GetObjectItem(component, "texture")->valuestring);

			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

			if (cJSON_GetObjectItem(component, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(component, "flags")->valuestring, NULL);
			}

			if (cJSON_GetObjectItem(component, "aiFlags"))
			{
				e->aiFlags = flagsToLong(cJSON_GetObjectItem(component, "aiFlags")->valuestring, NULL);
			}

			e->systemPower = MAX_SYSTEM_POWER;

			e->die = componentDie;

			e->owner = parent;

			component = component->next;

			parent->health++;
		}
	}

	parent->maxHealth = parent->health;
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
			e->health = e->maxHealth = cJSON_GetObjectItem(gun, "health")->valueint;
			e->reloadTime = cJSON_GetObjectItem(gun, "reloadTime")->valueint;
			e->offsetX = cJSON_GetObjectItem(gun, "x")->valueint;
			e->offsetY = cJSON_GetObjectItem(gun, "y")->valueint;
			e->texture = getTexture(cJSON_GetObjectItem(gun, "texture")->valuestring);
			e->guns[0].type = lookup(cJSON_GetObjectItem(gun, "type")->valuestring);
			e->missiles = getJSONValue(gun, "missiles", 0);

			if (cJSON_GetObjectItem(gun, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(gun, "flags")->valuestring, NULL);
			}

			if (cJSON_GetObjectItem(gun, "aiFlags"))
			{
				e->aiFlags = flagsToLong(cJSON_GetObjectItem(gun, "aiFlags")->valuestring, NULL);
			}

			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

			e->systemPower = MAX_SYSTEM_POWER;

			e->action = gunThink;
			e->die = gunDie;

			e->owner = parent;

			gun = gun->next;
		}
	}
}

static void loadEngines(Entity *parent, cJSON *engines)
{
	Entity *e;
	cJSON *engine;

	if (engines)
	{
		engine = engines->child;

		while (engine)
		{
			e = malloc(sizeof(Entity));
			memset(e, 0, sizeof(Entity));
			defTail->next = e;
			defTail = e;

			e->active = 1;

			e->type = ET_CAPITAL_SHIP_ENGINE;
			e->health = e->maxHealth = cJSON_GetObjectItem(engine, "health")->valueint;
			e->offsetX = cJSON_GetObjectItem(engine, "x")->valueint;
			e->offsetY = cJSON_GetObjectItem(engine, "y")->valueint;
			e->texture = getTexture(cJSON_GetObjectItem(engine, "texture")->valuestring);

			if (cJSON_GetObjectItem(engine, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(engine, "flags")->valuestring, NULL);
			}

			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

			e->systemPower = MAX_SYSTEM_POWER;

			e->action = engineThink;
			e->die = engineDie;

			e->owner = parent;

			engine = engine->next;
		}
	}
}

void updateCapitalShipComponentProperties(Entity *parent, long flags)
{
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->owner == parent)
		{
			switch (e->type)
			{
				case ET_CAPITAL_SHIP_ENGINE:
					sprintf(e->name, _("%s (Engine)"), parent->name);
					break;

				case ET_CAPITAL_SHIP_COMPONENT:
					sprintf(e->name, _("%s (Component)"), parent->name);
					break;

				case ET_CAPITAL_SHIP_GUN:
					sprintf(e->name, _("%s (Gun)"), parent->name);
					break;
			}

			e->active = parent->active;
			
			if (flags & EF_DISABLED)
			{
				e->flags |= EF_DISABLED;
			}
		}
	}
}

void loadCapitalShips(cJSON *node)
{
	Entity *e;
	char **types, *name, *groupName, *type;
	int side, scatter, number, active;
	int i, numTypes, addFlags;
	long flags;
	float x, y;

	if (node)
	{
		node = node->child;

		while (node)
		{
			name = NULL;
			groupName = NULL;
			flags = -1;

			types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
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
				type = types[rand() % numTypes];

				e = spawnCapitalShip(type, x, y, side);

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;

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

				updateCapitalShipComponentProperties(e, flags);
			}

			node = node->next;

			for (i = 0 ; i < numTypes ; i++)
			{
				free(types[i]);
			}

			free(types);
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
