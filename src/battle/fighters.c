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

#include "fighters.h"

static void separate(void);
static void die(void);
static void immediateDie(void);
static void spinDie(void);
static void straightDie(void);
static void randomizeDart(Entity *dart);
static void randomizeDartGuns(Entity *dart);
static void loadFighterDef(char *filename);

static Entity defHead, *defTail;

Entity *spawnFighter(char *name, int x, int y, int side)
{
	Entity *f, *def;
	
	f = spawnEntity();
	
	def = getFighterDef(name);
	
	memcpy(f, def, sizeof(Entity));
	
	f->id = battle.entId;
	f->next = NULL;
	
	f->x = x;
	f->y = y;
	f->side = side;
	
	switch (side)
	{
		case SIDE_ALLIES:
			f->aiAggression = rand() % 3;
			if (!(f->aiFlags & AIF_FOLLOWS_PLAYER))
			{
				f->aiFlags |= AIF_MOVES_TO_PLAYER;
			}
			break;
			
		case SIDE_PIRATE:
			f->aiAggression = rand() % 3;
			break;
			
		case SIDE_PANDORAN:
			f->aiAggression = 3 + rand() % 2;
			break;
	}
	
	if (strcmp(name, "ATAF") == 0)
	{
		f->aiAggression = 4;
	}
	
	if (strcmp(name, "Dart") == 0)
	{
		randomizeDart(f);
	}
	
	if (strcmp(name, "Civilian") == 0 && rand() % 2 == 0)
	{
		f->texture = getTexture("gfx/craft/civilian02.png");
	}
	
	if (f->aiFlags & AIF_AGGRESSIVE)
	{
		f->aiAggression = 4;
	}
	
	f->action = doAI;
	f->die = die;
	
	return f;
}

static void randomizeDart(Entity *dart)
{
	char textureName[MAX_DESCRIPTION_LENGTH];
	
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
	
	sprintf(textureName, "gfx/fighters/dart0%d.png", 1 + rand() % 7);
	
	dart->texture = getTexture(textureName);
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
		
		attachRope();
		
		self->reload = MAX(self->reload - 1, 0);
		self->shieldRecharge = MAX(self->shieldRecharge - 1, 0);
		self->armourHit = MAX(self->armourHit - 25, 0);
		self->shieldHit = MAX(self->shieldHit - 5, 0);
		self->systemHit = MAX(self->systemHit - 25, 0);
		
		if (self->thrust > 0.25)
		{
			addEngineEffect();
		}
		
		if (!self->shieldRecharge)
		{
			self->shield = MIN(self->shield + 1, self->maxShield);
			self->shieldRecharge = self->shieldRechargeRate;
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
				self->flags |= EF_DISABLED;
				updateObjective(self->name, TT_DISABLE);
				battle.stats[STAT_ENEMIES_DISABLED]++;
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
	}
	
	if (self->alive == ALIVE_ESCAPED)
	{
		if (self == player)
		{
			completeMission();
		}
		
		if (self->side != SIDE_ALLIES && (!(self->flags & EF_DISABLED)))
		{
			addHudMessage(colors.red, "Mission target has escaped.");
			battle.stats[STAT_ENEMIES_ESCAPED]++;
		}
		else if (strcmp(self->defName, "Civilian") == 0)
		{
			battle.stats[STAT_CIVILIANS_RESCUED]++;
		}
		
		updateObjective(self->name, TT_ESCAPED);
			
		updateCondition(self->name, TT_ESCAPED);
	}
	
	if (self->alive == ALIVE_DEAD)
	{
		if (self == player)
		{
			battle.stats[STAT_PLAYER_KILLED]++;
		}
		else if (player != NULL)
		{
			if (player->alive == ALIVE_ALIVE)
			{
				if (self->side != SIDE_ALLIES)
				{
					battle.stats[STAT_ENEMIES_KILLED]++;
				}
				else
				{
					if (strcmp(self->name, "Civilian") == 0)
					{
						battle.stats[STAT_CIVILIANS_KILLED]++;
						if (!battle.epic)
						{
							addHudMessage(colors.red, "Civilian has been killed");
						}
					}
					else
					{
						battle.stats[STAT_ALLIES_KILLED]++;
						if (!battle.epic)
						{
							addHudMessage(colors.red, "Ally has been killed");
						}
						
						runScriptFunction("ALLIES_KILLED %d", battle.stats[STAT_ALLIES_KILLED]);
					}
				}
			}
			
			updateObjective(self->name, TT_DESTROY);
			updateObjective(self->groupName, TT_DESTROY);
			
			adjustObjectiveTargetValue(self->name, TT_ESCAPED, -1);
			
			updateCondition(self->name, TT_DESTROY);
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
	
	candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_FIGHTER)
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

void drawFighter(Entity *e)
{
	SDL_SetTextureColorMod(e->texture, 255, 255, 255);
	
	if (e->armourHit > 0)
	{
		SDL_SetTextureColorMod(e->texture, 255, 255 - e->armourHit, 255 - e->armourHit);
	}
	
	if (e->systemHit > 0)
	{
		SDL_SetTextureColorMod(e->texture, 255 - e->systemHit, 255, 255);
	}
	
	blitRotated(e->texture, e->x - battle.camera.x, e->y - battle.camera.y, e->angle);
	
	if (e->shieldHit > 0)
	{
		drawShieldHitEffect(e);
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

void damageFighter(Entity *f, int amount, long flags)
{
	int prevShield = f->shield;
	
	if (flags & BF_SYSTEM_DAMAGE)
	{
		f->systemPower = MAX(0, f->systemPower - amount);
		
		f->systemHit = 255;
	
		if (f->systemPower == 0)
		{
			f->shield = f->maxShield = 0;
			f->action = NULL;
		}
	}
	else if (flags & BF_SHIELD_DAMAGE)
	{
		f->shield = MAX(-(FPS * 10), f->shield - amount);
		
		if (f->shield <= 0 && prevShield > 0)
		{
			playBattleSound(SND_SHIELD_BREAK, f->x, f->y);
			addShieldSplinterEffect(f);
		}
	}
	else
	{
		if (f->shield > 0)
		{
			f->shield -= amount;
			
			if (f->shield < 0)
			{
				f->health += f->shield;
				f->shield = 0;
			}
		}
		else
		{
			f->health -= amount;
			f->armourHit = 255;
			
			playBattleSound(SND_ARMOUR_HIT, f->x, f->y);
		}
	}
	
	if (f->shield > 0)
	{
		f->shieldHit = 255;
		
		playBattleSound(SND_SHIELD_HIT, f->x, f->y);
	}
}

static void die(void)
{
	int n = rand() % 3;
	if (self == player)
	{
		n = rand() % 2;
	}
	else if (self->aiFlags & AIF_INSTANT_DIE)
	{
		n = 2;
	}
	
	switch (n)
	{
		case 0:
			self->action = straightDie;
			break;
			
		case 1:
			self->action = spinDie;
			break;
			
		case 2:
			self->action = immediateDie;
			break;
	}
}

static void immediateDie(void)
{
	self->alive = ALIVE_DEAD;
	addFighterExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
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
		addFighterExplosion();
		playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
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
		addFighterExplosion();
		playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	}
}

void retreatEnemies(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->type == ET_FIGHTER && e->side != SIDE_ALLIES)
		{
			e->aiFlags |= AIF_AVOIDS_COMBAT;
		}
	}
}

void retreatAllies(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->type == ET_FIGHTER && e->side == SIDE_ALLIES)
		{
			e->flags |= EF_RETREATING;
			
			e->aiFlags |= AIF_AVOIDS_COMBAT;
			e->aiFlags |= AIF_UNLIMITED_RANGE;
			e->aiFlags |= AIF_GOAL_EXTRACTION;
			e->aiFlags &= ~AIF_FOLLOWS_PLAYER;
			e->aiFlags &= ~AIF_MOVES_TO_PLAYER;
		}
	}
}

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
		f->missiles = cJSON_GetObjectItem(root, "missiles")->valueint;
	}
	
	if (cJSON_GetObjectItem(root, "flags"))
	{
		f->flags = flagsToLong(cJSON_GetObjectItem(root, "flags")->valuestring);
	}
	
	if (cJSON_GetObjectItem(root, "aiFlags"))
	{
		f->aiFlags = flagsToLong(cJSON_GetObjectItem(root, "aiFlags")->valuestring);
	}
	
	f->separationRadius = MAX(f->w, f->h);
	f->separationRadius *= 3;
	
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
