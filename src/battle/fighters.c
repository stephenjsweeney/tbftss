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

#include "fighters.h"

static void separate(void);
static void die(void);
static void immediateDie(void);
static void spinDie(void);
static void straightDie(void);
static void simpleDie(void);
static void randomizeDart(Entity *dart);
static void randomizeDartGuns(Entity *dart);
static void loadFighterDef(char *filename);
static void loadFighterDefList(char *filename);
static Entity *getFighterDef(char *name);
static void addFighterStat(char *name);
static void incFighterStat(char *key);

static Entity defHead, *defTail;

Entity *spawnFighter(char *name, int x, int y, int side)
{
	Entity *e, *def;

	e = spawnEntity();

	def = getFighterDef(name);

	memcpy(e, def, sizeof(Entity));

	e->next = NULL;

	e->x = x;
	e->y = y;
	e->side = side;

	switch (side)
	{
		case SIDE_ALLIES:
			e->aiAggression = 2 + rand() % 2;
			if (!(e->aiFlags & AIF_FOLLOWS_PLAYER))
			{
				e->aiFlags |= AIF_MOVES_TO_PLAYER;
			}
			break;

		case SIDE_PIRATE:
			e->aiAggression = rand() % 3;
			break;

		case SIDE_PANDORAN:
			e->aiAggression = 3 + rand() % 2;
			break;

		case SIDE_TZAC:
			e->aiAggression = 1 + rand() % 3;
			break;
	}

	if (strcmp(name, "ATAF") == 0)
	{
		e->aiAggression = 4;
	}

	if (strcmp(name, "Dart") == 0)
	{
		randomizeDart(e);
	}

	if (strcmp(name, "Civilian") == 0 && rand() % 2 == 0)
	{
		e->texture = getTexture("gfx/craft/civilian02.png");
	}

	if (e->aiFlags & AIF_AGGRESSIVE)
	{
		e->aiAggression = 4;
	}

	e->action = doAI;
	e->die = die;
	
	if (game.currentMission->challengeData.isDeathMatch)
	{
		e->side = SDL_GetTicks();
	}

	return e;
}

static void randomizeDart(Entity *dart)
{
	char texture[MAX_DESCRIPTION_LENGTH];

	if (rand() % 5 == 0)
	{
		dart->health = dart->maxHealth = 5 + (rand() % 21);
	}

	if (rand() % 5 == 0)
	{
		dart->shield = dart->maxShield = 1 + (rand() % 16);
		dart->shieldRechargeRate = 30 + (rand() % 90);
	}

	if (rand() % 5 == 0)
	{
		dart->speed = 2 + (rand() % 3);
	}

	if (rand() % 5 == 0)
	{
		dart->reloadTime = 24 + (rand() % 11);
	}

	randomizeDartGuns(dart);

	dart->missiles = rand() % 3;

	sprintf(texture, "gfx/fighters/dart0%d.png", 1 + rand() % 7);

	dart->texture = getTexture(texture);
}

static void randomizeDartGuns(Entity *dart)
{
	int i;

	switch (rand() % 4)
	{
		/* Single plasma gun */
		case 0:
			dart->guns[0].type = BT_PLASMA;
			dart->guns[0].x = dart->guns[0].y = 0;

			for (i = 1 ; i < MAX_FIGHTER_GUNS ; i++)
			{
				if (dart->guns[i].type)
				{
					dart->guns[i].type = BT_NONE;
				}
			}
			break;

		/* Dual plasma guns */
		case 1:
			dart->guns[0].type = BT_PLASMA;
			dart->guns[1].type = BT_PLASMA;
			break;

		/* Triple particle guns */
		case 2:
			dart->guns[2].type = BT_PARTICLE;
			dart->guns[2].y = -10;
			break;


		/* Plasma / Laser cannons */
		case 3:
			dart->guns[0].type = BT_PLASMA;
			dart->guns[0].x = dart->guns[0].y = 0;

			dart->guns[1].type = BT_LASER;
			dart->guns[1].x = dart->guns[1].y = 0;
			break;

		/* Dual Laser cannons */
		case 4:
			dart->guns[0].type = BT_LASER;
			dart->guns[1].type = BT_LASER;
			break;
	}
}

void doFighter(void)
{
	if (self->alive == ALIVE_ALIVE)
	{
		if (self != player)
		{
			separate();
		}

		if (!(self->flags & EF_DISABLED))
		{
			attachRope();
		}

		if (self->thrust > 0.25)
		{
			addEngineEffect();
		}

		if (self->health <= 0)
		{
			self->health = 0;
			self->alive = ALIVE_DYING;
			self->die();

			if (self == battle.missionTarget)
			{
				battle.missionTarget = NULL;
			}
		}
		else if (self->systemPower <= 0 || (self->flags & EF_DISABLED))
		{
			self->dx *= 0.99;
			self->dy *= 0.99;
			self->thrust = 0;
			self->shield = self->maxShield = 0;
			self->action = NULL;

			if ((self->flags & EF_DISABLED) == 0)
			{
				playBattleSound(SND_POWER_DOWN, self->x, self->y);

				self->flags |= EF_DISABLED;
				self->flags |= EF_SECONDARY_TARGET;

				battle.stats[STAT_ENEMIES_DISABLED]++;

				updateObjective(self->name, TT_DISABLE);
				updateObjective(self->groupName, TT_DISABLE);
				
				if (self->side != player->side)
				{
					runScriptFunction("ENEMIES_DISABLED %d", battle.stats[STAT_ENEMIES_DISABLED]);
				}
			}
		}

		if (self->target != NULL && self->target->alive != ALIVE_ALIVE)
		{
			self->target = NULL;

			if (self != player)
			{
				self->action = doAI;
			}
		}
		
		if (self->aiFlags & AIF_SUSPICIOUS)
		{
			checkSuspicionLevel();
		}
		
		if (self->aiFlags & AIF_ZAK_SUSPICIOUS)
		{
			checkZackariaSuspicionLevel();
		}
	}

	if (self->alive == ALIVE_ESCAPED)
	{
		if (self == player && !game.currentMission->challengeData.isChallenge)
		{
			updateObjective("Player", TT_ESCAPED);
			
			completeMission();
		}

		if (self->side != player->side && (!(self->flags & EF_DISABLED)))
		{
			addHudMessage(colors.red, _("Mission target has escaped."));
			battle.stats[STAT_ENEMIES_ESCAPED]++;
		}

		if (strcmp(self->defName, "Civilian") == 0)
		{
			battle.stats[STAT_CIVILIANS_RESCUED]++;
		}

		/* if you did not escape under your own volition, or with the aid of a friend, you've been stolen */
		if (!self->owner || self->side == self->owner->side)
		{
			updateObjective(self->name, TT_ESCAPED);
			updateCondition(self->name, TT_ESCAPED);
		}
		else
		{
			updateObjective(self->name, TT_STOLEN);
			updateCondition(self->name, TT_STOLEN);
		}
	}

	if (self->alive == ALIVE_DEAD)
	{
		if (player->alive == ALIVE_ALIVE && self != player)
		{
			if (self->side != player->side)
			{
				if (!(self->flags & EF_NO_KILL_INC))
				{
					battle.stats[STAT_ENEMIES_KILLED]++;

					runScriptFunction("ENEMIES_KILLED %d", battle.stats[STAT_ENEMIES_KILLED]);
				}
			}
			else
			{
				if (strcmp(self->name, "Civilian") == 0)
				{
					battle.stats[STAT_CIVILIANS_KILLED]++;
					if (!battle.isEpic || game.currentMission->challengeData.isChallenge)
					{
						addHudMessage(colors.red, _("Civilian has been killed"));
					}
					
					runScriptFunction("CIVILIANS_KILLED %d", battle.stats[STAT_CIVILIANS_KILLED]);
				}
				else
				{
					battle.stats[STAT_ALLIES_KILLED]++;
					if (!battle.isEpic && !game.currentMission->challengeData.isChallenge)
					{
						addHudMessage(colors.red, _("Ally has been killed"));
					}

					runScriptFunction("ALLIES_KILLED %d", battle.stats[STAT_ALLIES_KILLED]);
				}
			}
			
			updateObjective(self->name, TT_DESTROY);
			updateObjective(self->groupName, TT_DESTROY);

			adjustObjectiveTargetValue(self->name, TT_ESCAPED, -1);

			updateCondition(self->name, TT_DESTROY);
			updateCondition(self->groupName, TT_DESTROY);
			
			/* don't fire if the opposing side is responsible */
			if (self->flags & EF_SURRENDERED && self->killedBy->side == player->side)
			{
				updateCondition("SURRENDERED", TT_DESTROY);
			}
		}
	}
}

static void separate(void)
{
	int angle;
	int distance;
	float dx, dy, force;
	int count;
	Entity *e, **candidates;
	int i;

	dx = dy = 0;
	count = 0;
	force = 0;

	candidates = getAllEntsInRadius(self->x, self->y, self->separationRadius, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if ((e->flags & EF_TAKES_DAMAGE) && (!(e->flags & EF_NON_SOLID)))
		{
			distance = getDistance(e->x, e->y, self->x, self->y);

			if (distance > 0 && distance < self->separationRadius)
			{
				angle = getAngle(self->x, self->y, e->x, e->y);

				dx += sin(TO_RAIDANS(angle));
				dy += -cos(TO_RAIDANS(angle));
				force += (self->separationRadius - distance) * 0.005;

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

		self->dx -= dx;
		self->dy -= dy;
	}
}

void applyFighterThrust(void)
{
	float v;

	self->dx += sin(TO_RAIDANS(self->angle)) * 0.1;
	self->dy += -cos(TO_RAIDANS(self->angle)) * 0.1;
	self->thrust = sqrt((self->dx * self->dx) + (self->dy * self->dy));

	if (self->thrust > self->speed * self->speed)
	{
		v = (self->speed / sqrt(self->thrust));
		self->dx = v * self->dx;
		self->dy = v * self->dy;
		self->thrust = sqrt((self->dx * self->dx) + (self->dy * self->dy));
	}
}

void applyFighterBrakes(void)
{
	self->dx *= 0.95;
	self->dy *= 0.95;

	self->thrust = sqrt((self->dx * self->dx) + (self->dy * self->dy));
}

void damageFighter(Entity *e, int amount, long flags)
{
	int prevShield = e->shield;

	e->aiDamageTimer = FPS;
	e->aiDamagePerSec += amount;
	
	if (flags & BF_SYSTEM_DAMAGE)
	{
		if (e->shield > 0)
		{
			amount /= 2;
			
			e->shield -= amount;
			
			if (e->shield < 0)
			{
				amount = -e->shield;
			}
		}
		
		if (amount >= 0)
		{
			e->systemPower = MAX(0, e->systemPower - amount);

			e->systemHit = 255;

			if (e->systemPower == 0)
			{
				e->shield = e->maxShield = 0;
				e->action = NULL;
			}
			
			playBattleSound(SND_MAG_HIT, e->x, e->y);
		}
	}
	else if (flags & BF_SHIELD_DAMAGE)
	{
		e->shield -= amount;

		if (e->shield <= 0 && prevShield > 0)
		{
			playBattleSound(SND_SHIELD_BREAK, e->x, e->y);
			addShieldSplinterEffect(e);
			e->shield = -(FPS * 10);
		}

		e->shield = MAX(-(FPS * 10), e->shield);
	}
	else
	{
		if (e->shield > 0)
		{
			e->shield -= amount;
			
			if (e->shield <= 0)
			{
				e->armourHit = 255;
				e->health += e->shield;
				e->shield = 0;
				
				playBattleSound(SND_ARMOUR_HIT, e->x, e->y);
			}
		}
		else
		{
			e->health -= amount;
			e->armourHit = 255;

			playBattleSound(SND_ARMOUR_HIT, e->x, e->y);
		}
	}

	if (e->shield > 0)
	{
		e->shieldHit = 255;

		playBattleSound(SND_SHIELD_HIT, e->x, e->y);
	}
	
	/* don't allow the shield to recharge immediately after taking a hit */
	e->shieldRecharge = e->shieldRechargeRate;

	/*
	 * Sometimes run away if you take too much damage in a short space of time
	 */
	if (e->type == ET_FIGHTER && (!(e->aiFlags & AIF_EVADE)) && e != player && e->aiDamagePerSec >= (e->maxHealth + e->maxShield) * 0.1)
	{
		if ((rand() % 10) > 7)
		{
			e->action = doAI;
			e->aiFlags |= AIF_EVADE;
			e->aiActionTime = e->aiEvadeTimer = FPS * (1 + (rand() % 3));
		}
		else
		{
			e->aiDamagePerSec = 0;
		}
	}
}

static void die(void)
{
	int n = rand() % 3;

	switch (self->deathType)
	{
		case DT_ANY:
			n = rand() % 3;
			break;
		case DT_NO_SPIN:
			n = 1 + rand() % 2;
			break;
		case DT_INSTANT:
			n = 2;
			break;
		case DT_SIMPLE:
			n = 3;
			break;
	}

	if (self == player && battle.isEpic)
	{
		n = 1;
	}

	switch (n)
	{
		case 0:
			self->action = spinDie;
			break;
		case 1:
			self->action = straightDie;
			break;
		case 2:
			self->action = immediateDie;
			break;
		case 3:
			self->action = simpleDie;
			break;
	}
	
	if (self->killedBy == player && (!(self->flags & EF_NO_KILL_INC)))
	{
		battle.stats[STAT_ENEMIES_KILLED_PLAYER]++;
		
		if (self->flags & EF_COMMON_FIGHTER)
		{
			incFighterStat(self->defName);
		}
		
		if (battle.isEpic)
		{
			battle.stats[STAT_EPIC_KILL_STREAK]++;
		}
	}
	
	if (self->flags & EF_DROPS_ITEMS)
	{
		addRandomItem(self->x, self->y);
	}
}

static void immediateDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	addDebris(self->x, self->y, 3 + rand() % 6);
}

static void spinDie(void)
{
	self->health--;
	self->thinkTime = 0;
	self->armourHit = 0;
	self->shieldHit = 0;
	self->systemHit = 0;

	self->angle += 8;

	if (rand() % 2 == 0)
	{
		addSmallFighterExplosion();
	}

	if (self->health <= -(FPS * 1.5))
	{
		self->alive = ALIVE_DEAD;
		addSmallExplosion();
		playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
		addDebris(self->x, self->y, 3 + rand() % 6);
	}
}

static void straightDie(void)
{
	self->health--;
	self->thinkTime = 0;
	self->armourHit = 0;
	self->shieldHit = 0;
	self->systemHit = 0;

	if (rand() % 2 == 0)
	{
		addSmallFighterExplosion();
	}

	if (self->health <= -(FPS * 1.5))
	{
		self->alive = ALIVE_DEAD;
		addSmallExplosion();
		playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
		addDebris(self->x, self->y, 3 + rand() % 6);
	}
}

static void simpleDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
}

void retreatEnemies(void)
{
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->type == ET_FIGHTER && e->side != player->side)
		{
			e->flags |= EF_RETREATING;
			
			e->aiFlags |= AIF_AVOIDS_COMBAT;
			e->aiFlags |= AIF_UNLIMITED_RANGE;
			e->aiFlags &= ~AIF_MOVES_TO_LEADER;
			e->aiFlags &= ~AIF_WANDERS;
			
			e->aiActionTime = MIN(e->aiActionTime, FPS);
			
			if (!game.currentMission->challengeData.isChallenge)
			{
				e->aiFlags |= AIF_GOAL_JUMPGATE;
			}
		}
	}
}

void retreatAllies(void)
{
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->type == ET_FIGHTER && e->side == player->side)
		{
			e->flags |= EF_RETREATING;

			e->aiFlags |= AIF_AVOIDS_COMBAT;
			e->aiFlags |= AIF_UNLIMITED_RANGE;
			e->aiFlags &= ~AIF_FOLLOWS_PLAYER;
			e->aiFlags &= ~AIF_MOVES_TO_PLAYER;
			e->aiFlags &= ~AIF_MOVES_TO_LEADER;
			e->aiFlags &= ~AIF_WANDERS;
			
			e->aiActionTime = MIN(e->aiActionTime, FPS);
			
			if (!game.currentMission->challengeData.isChallenge)
			{
				e->aiFlags |= AIF_GOAL_JUMPGATE;
			}
		}
	}
}

static Entity *getFighterDef(char *name)
{
	Entity *e;

	for (e = defHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, name) == 0)
		{
			return e;
		}
	}

	printf("Error: no such fighter '%s'\n", name);
	exit(1);
}

void loadFighterDefs(void)
{
	memset(&defHead, 0, sizeof(Entity));
	defTail = &defHead;

	loadFighterDefList("data/fighters");
	loadFighterDefList("data/craft");
	loadFighterDefList("data/turrets");
}

static void loadFighterDefList(char *dir)
{
	char **filenames;
	char path[MAX_FILENAME_LENGTH];
	int count, i;

	filenames = getFileList(dir, &count);

	for (i = 0 ; i < count ; i++)
	{
		sprintf(path, "%s/%s", dir, filenames[i]);

		loadFighterDef(path);

		free(filenames[i]);
	}

	free(filenames);
}

static void loadFighterDef(char *filename)
{
	cJSON *root, *node;
	char *text;
	Entity *e;
	int i;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);

	root = cJSON_Parse(text);

	if (root)
	{
		e = malloc(sizeof(Entity));
		memset(e, 0, sizeof(Entity));
		defTail->next = e;
		defTail = e;

		e->type = ET_FIGHTER;
		e->active = 1;

		STRNCPY(e->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(e->defName, e->name, MAX_NAME_LENGTH);
		e->health = e->maxHealth = cJSON_GetObjectItem(root, "health")->valueint;
		e->shield = e->maxShield = getJSONValue(root, "shield", 0);
		e->speed = cJSON_GetObjectItem(root, "speed")->valuedouble;
		e->reloadTime = getJSONValue(root, "reloadTime", 0);
		e->shieldRechargeRate = getJSONValue(root, "shieldRechargeRate", 0);
		e->texture = getTexture(cJSON_GetObjectItem(root, "texture")->valuestring);

		SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);

		if (cJSON_GetObjectItem(root, "guns"))
		{
			i = 0;

			for (node = cJSON_GetObjectItem(root, "guns")->child ; node != NULL ; node = node->next)
			{
				e->guns[i].type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
				e->guns[i].x = cJSON_GetObjectItem(node, "x")->valueint;
				e->guns[i].y = cJSON_GetObjectItem(node, "y")->valueint;

				i++;

				if (i >= MAX_FIGHTER_GUNS)
				{
					printf("ERROR: cannot assign more than %d guns to a fighter\n", MAX_FIGHTER_GUNS);
					exit(1);
				}
			}

			e->combinedGuns = getJSONValue(root, "combinedGuns", 0);
		}

		e->selectedGunType = e->guns[0].type;

		e->missiles = getJSONValue(root, "missiles", 0);

		if (cJSON_GetObjectItem(root, "flags"))
		{
			e->flags = flagsToLong(cJSON_GetObjectItem(root, "flags")->valuestring, NULL);
		}

		if (cJSON_GetObjectItem(root, "aiFlags"))
		{
			e->aiFlags = flagsToLong(cJSON_GetObjectItem(root, "aiFlags")->valuestring, NULL);
		}

		if (cJSON_GetObjectItem(root, "deathType"))
		{
			e->deathType = lookup(cJSON_GetObjectItem(root, "deathType")->valuestring);
		}
		
		if (e->flags & EF_COMMON_FIGHTER)
		{
			addFighterStat(e->name);
		}

		e->separationRadius = MAX(e->w, e->h) * 3;

		e->systemPower = MAX_SYSTEM_POWER;

		cJSON_Delete(root);
	}
	else
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Failed to load '%s'", filename);
	}

	free(text);
}

static void addFighterStat(char *key)
{
	Tuple *t, *tail;
	
	tail = &game.fighterStatHead;
	
	for (t = game.fighterStatHead.next ; t != NULL ; t = t->next)
	{
		if (strcmp(t->key, key) == 0)
		{
			return;
		}
		
		tail = t;
	}
	
	t = malloc(sizeof(Tuple));
	memset(t, 0, sizeof(Tuple));
	
	STRNCPY(t->key, key, MAX_NAME_LENGTH);
	t->value = 0;
	
	tail->next = t;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Added '%s' to fighter stats", key);
}

static void incFighterStat(char *key)
{
	Tuple *t;
	
	for (t = game.fighterStatHead.next ; t != NULL ; t = t->next)
	{
		if (strcmp(t->key, key) == 0)
		{
			t->value++;
			return;
		}
	}
}

void loadFighters(cJSON *node)
{
	Entity *e;
	char **types, *name, *groupName, *type, *strpos;
	int side, scatter, number, active;
	int i, numTypes, addFlags, addAIFlags, id;
	long flags, aiFlags;
	float x, y;

	if (node)
	{
		id = 0;
		
		node = node->child;

		while (node)
		{
			name = NULL;
			groupName = NULL;
			flags = -1;
			aiFlags = -1;

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

			if (cJSON_GetObjectItem(node, "aiFlags"))
			{
				aiFlags = flagsToLong(cJSON_GetObjectItem(node, "aiFlags")->valuestring, &addAIFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				type = types[rand() % numTypes];

				e = spawnFighter(type, x, y, side);

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;

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
					
					if (e->flags & EF_DISABLED)
					{
						e->speed = 0;
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

						SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "AI Flags for '%s' (%s) replaced", e->name, e->defName);
					}
				}

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
					
					/* update 'name #?' to 'name #1', etc. */
					strpos = strstr(e->name, "#?");
					
					if (strpos)
					{
						*(++strpos) = ('0' + ++id);
					}
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}
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

void destroyFighterDefs(void)
{
	Entity *e;

	while (defHead.next)
	{
		e = defHead.next;
		defHead.next = e->next;
		free(e);
	}
}
