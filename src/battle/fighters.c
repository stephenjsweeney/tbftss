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
static void checkHasFled(void);

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
			f->aggression = 1 + rand() % 3;
			break;
			
		case SIDE_PIRATE:
			f->aggression = rand() % 3;
			break;
			
		case SIDE_PANDORAN:
			f->aggression = 3 + rand() % 2;
			break;
	}
	
	if (strcmp(name, "ATAF") == 0)
	{
		f->aggression = 4;
	}
	
	if (strcmp(name, "Dart") == 0)
	{
		randomizeDart(f);
	}
	
	if (strcmp(name, "Civilian") == 0 && rand() % 2 == 0)
	{
		f->texture = getTexture("gfx/craft/civilian02.png");
	}
	
	if (f->flags & EF_CIVILIAN)
	{
		f->defaultAction = doCivilianAI;
	}
	else
	{
		f->defaultAction = doAI;
	}
	
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
	
	dart->missiles.ammo = rand() % 3;
	
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
			
			for (i = 2 ; i < MAX_FIGHTER_GUNS ; i++)
			{
				if (dart->guns[i].type)
				{
					dart->guns[i].type = BT_NONE;
				}
			}
			break;
		
		/* Triple particle guns */
		case 2:
			dart->guns[2].type = BT_PARTICLE;
			dart->guns[2].y = -10;
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
		
		if (self->action == NULL && self->defaultAction != NULL)
		{
			self->action = self->defaultAction;
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
		else if (self->systemPower <= 0)
		{
			self->dx *= 0.99;
			self->dy *= 0.99;
			self->thrust = 0;
			self->shield = self->maxShield = 0;
			self->action = NULL;
			
			if (self->alive == ALIVE_ALIVE)
			{
				updateObjective(self->name, TT_DISABLE);
				battle.stats[STAT_DISABLED]++;
			}
		}
		else
		{
			if (player != NULL && self->flags & EF_FLEEING && battle.stats[STAT_TIME] % FPS == 0)
			{
				checkHasFled();
			}
		}
	}
	
	if (self->alive == ALIVE_ESCAPED)
	{
		if (self->side != SIDE_ALLIES)
		{
			addHudMessage(colors.red, "Mission target has escaped.");
			battle.stats[STAT_ENEMIES_ESCAPED]++;
		}
		else
		{
			battle.stats[STAT_ALLIES_ESCAPED]++;
		}
		
		updateObjective(self->name, TT_ESCAPED);
			
		updateCondition(self->name, TT_ESCAPED);
		
		checkTrigger("ESCAPE", TRIGGER_ESCAPES);
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
					battle.stats[STAT_ALLIES_KILLED]++;
					
					if (!battle.epic)
					{
						addHudMessage(colors.red, "Ally has been killed");
					}
				}
			}
			
			updateObjective(self->name, TT_DESTROY);
			
			updateCondition(self->name, TT_DESTROY);
			
			checkTrigger(self->name, TRIGGER_KILLS);
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
	
	candidates = getAllEntsWithin(self->x, self->y, self->w, self->h, self);
	
	for (i = 0, e = candidates[i] ; e != NULL ; i++, e = candidates[i])
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
	SDL_Texture *shieldHitTexture = getTexture("gfx/battle/shieldHit.png");
	
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
		SDL_SetTextureBlendMode(shieldHitTexture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(shieldHitTexture, e->shieldHit);
		blit(shieldHitTexture, e->x - battle.camera.x, e->y - battle.camera.y, 1);
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
	if (flags & BF_SYSTEM_DAMAGE)
	{
		f->systemPower = MAX(0, f->systemPower - amount);
		
		f->systemHit = 255;
	
		if (f->systemPower == 0)
		{
			f->shield = f->maxShield = 0;
			f->action = f->defaultAction = NULL;
		}
	}
	else
	{
		f->shield -= amount;
		
		if (f->shield < 0)
		{
			f->health -= abs(f->shield);
			f->shield = 0;
			f->armourHit = 255;
			
			playBattleSound(SND_ARMOUR_HIT, f->x, f->y);
		}
		else if (f->shield > 0)
		{
			f->shieldHit = 255;
		}
	}
}

static void die(void)
{
	int n = rand() % 3;
	if (self == player)
	{
		n = rand() % 2;
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

static void checkHasFled(void)
{
	long distance = getDistance(self->x, self->y, player->x, player->y);
	
	if (distance > 5000)
	{
		self->alive = ALIVE_ESCAPED;
	}
}
