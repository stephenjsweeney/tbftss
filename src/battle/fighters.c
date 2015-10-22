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
static void randomizeDart(Fighter *dart);
static void randomizeDartGuns(Fighter *dart);

Fighter *spawnFighter(char *name, int x, int y, int side)
{
	Fighter *f, *def;
	
	f = malloc(sizeof(Fighter));
	memset(f, 0, sizeof(Fighter));
	
	def = getFighterDef(name);
	
	memcpy(f, def, sizeof(Fighter));
	
	f->next = NULL;
	
	battle.fighterTail->next = f;
	battle.fighterTail = f;
	
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
	
	f->defaultAction = doAI;
	f->die = die;
	
	return f;
}

static void randomizeDart(Fighter *dart)
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
	
	sprintf(textureName, "gfx/fighters/dart0%d.png", 1 + rand() % 7);
	
	dart->texture = getTexture(textureName);
}

static void randomizeDartGuns(Fighter *dart)
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

void doFighters(void)
{
	Fighter *f, *prev;
	
	battle.numAllies = battle.numEnemies = 0;
	
	prev = &battle.fighterHead;
	
	for (f = battle.fighterHead.next ; f != NULL ; f = f->next)
	{
		self = f;
		
		if (player != NULL)
		{
			if (f != player && f->health > 0)
			{
				separate();
			}
			
			if (f->side == player->side)
			{
				battle.numAllies++;
			}
			else
			{
				battle.numEnemies++;
			}
		}
		
		if (self->target != NULL && self->target->health <= 0)
		{
			self->action = self->defaultAction;
			self->target = NULL;
		}
		
		f->x += f->dx;
		f->y += f->dy;
		
		if (f != player)
		{
			f->x -= battle.ssx;
			f->y -= battle.ssy;
		}
		
		if (f->health > 0)
		{
			f->reload = MAX(f->reload - 1, 0);
			f->shieldRecharge = MAX(f->shieldRecharge - 1, 0);
			f->armourHit = MAX(f->armourHit - 25, 0);
			f->shieldHit = MAX(f->shieldHit - 5, 0);
			f->systemHit = MAX(f->systemHit - 25, 0);
			
			if (self->thrust > 0.25)
			{
				addEngineEffect();
			}
			
			if (!f->shieldRecharge)
			{
				f->shield = MIN(f->shield + 1, f->maxShield);
				f->shieldRecharge = f->shieldRechargeRate;
			}
			
			if (f->action == NULL && f->defaultAction != NULL)
			{
				f->action = f->defaultAction;
			}
		}
		
		if (f->action != NULL)
		{
			if (--f->thinkTime <= 0)
			{
				f->thinkTime = 0;
				f->action();
			}
		}
		
		if (f->health <= 0 && f->alive == ALIVE_ALIVE)
		{
			f->health = 0;
			f->alive = ALIVE_DYING;
			f->die();
		}
		
		if (f->systemPower <= 0)
		{
			f->dx *= 0.99;
			f->dy *= 0.99;
			f->thrust = 0;
			f->shield = f->maxShield = 0;
			f->action = NULL;
			
			if (f->alive == ALIVE_ALIVE)
			{
				updateObjective(f->name, TT_DISABLE);
				f->alive = ALIVE_DISABLED;
			}
		}
		
		if (f->alive == ALIVE_DEAD)
		{
			if (f == player)
			{
				battle.stats.playerKilled++;
			}
			else if (player != NULL)
			{
				if (player->alive == ALIVE_ALIVE)
				{
					if (f->side != player->side)
					{
						battle.stats.enemiesKilled++;
					}
					else
					{
						battle.stats.alliesKilled++;
						
						addHudMessage(colors.red, "Ally has been killed");
					}
				}
				
				updateObjective(f->name, TT_DESTROY);
				
				updateCondition(f->name, TT_DESTROY);
			}
			
			if (f == battle.fighterTail)
			{
				battle.fighterTail = prev;
			}
			
			if (f == player)
			{
				player = NULL;
			}
			
			prev->next = f->next;
			free(f);
			f = prev;
		}
		
		prev = f;
	}
}

static void separate(void)
{
	int angle;
	int distance;
	float dx, dy, force;
	int count;
	Fighter *f;
	
	dx = dy = 0;
	count = 0;
	force = 0;
	
	for (f = battle.fighterHead.next ; f != NULL ; f = f->next)
	{
		if (f != self)
		{
			distance = getDistance(f->x, f->y, self->x, self->y);
			
			if (distance > 0 && distance < self->separationRadius)
			{
				angle = getAngle(self->x, self->y, f->x, f->y);
				
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

void drawFighters(void)
{
	Fighter *f;
	SDL_Rect r;
	SDL_Texture *shieldHitTexture = getTexture("gfx/battle/shieldHit.png");
	
	for (f = battle.fighterHead.next ; f != NULL ; f = f->next)
	{
		SDL_SetTextureColorMod(f->texture, 255, 255, 255);
		
		if (f->armourHit > 0)
		{
			SDL_SetTextureColorMod(f->texture, 255, 255 - f->armourHit, 255 - f->armourHit);
		}
		
		if (f->systemHit > 0)
		{
			SDL_SetTextureColorMod(f->texture, 255 - f->systemHit, 255, 255);
		}
		
		blitRotated(f->texture, f->x, f->y, f->angle);
		
		if (f->shieldHit > 0)
		{
			SDL_SetTextureBlendMode(shieldHitTexture, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(shieldHitTexture, f->shieldHit);
			blit(shieldHitTexture, f->x, f->y, 1);
		}
		
		if (player != NULL && f == player->target)
		{
			r.x = f->x - 32;
			r.y = f->y - 32;
			r.w = 64;
			r.h = 64;
			
			SDL_SetRenderDrawColor(app.renderer, 255, 64, 0, 255);
			SDL_RenderDrawRect(app.renderer, &r);
		}
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

void damageFighter(Fighter *f, int amount, long flags)
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
	
	if (self->health <= -FPS)
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
	
	if (self->health <= -FPS)
	{
		self->alive = ALIVE_DEAD;
		addFighterExplosion();
		playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	}
}
