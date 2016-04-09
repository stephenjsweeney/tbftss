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

#include "mine.h"

static void think(void);
static void die(void);
static void lookForFighters(void);
static void doSplashDamage(void);

static SDL_Texture *mineWarning = NULL;
static SDL_Texture *mineNormal = NULL;

Entity *spawnMine(void)
{
	Entity *mine = spawnEntity();
	
	if (!mineNormal || !mineWarning)
	{
		mineNormal = getTexture("gfx/entities/mine.png");
		mineWarning = getTexture("gfx/entities/mineWarning.png");
	}

	mine->type = ET_MINE;
	mine->health = mine->maxHealth = 1;
	mine->speed = 1;
	mine->systemPower = SYSTEM_POWER;
	mine->texture = mineNormal;
	mine->action = think;
	mine->die = die;
	mine->flags = EF_TAKES_DAMAGE+EF_NO_PLAYER_TARGET;
	
	SDL_QueryTexture(mine->texture, NULL, NULL, &mine->w, &mine->h);

	return mine;
}

static void think(void)
{
	self->texture = mineNormal;
	
	self->angle += 0.1;
	
	if (self->angle >= 360)
	{
		self->angle -= 360;
	}
	
	self->dx *= 0.99;
	self->dy *= 0.99;
	
	lookForFighters();
	
	if (self->systemPower < SYSTEM_POWER && battle.stats[STAT_TIME] % 8 < 4)
	{
		playBattleSound(SND_MINE_WARNING, self->x, self->y);
		
		self->texture = mineWarning;
	}
}

static void lookForFighters(void)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsWithin(self->x - (self->w / 2) - DAMAGE_RANGE, self->y - (self->h / 2) - DAMAGE_RANGE, self->w + DAMAGE_RANGE, self->h + DAMAGE_RANGE, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->side != self->side && e->health > 0 && e->type == ET_FIGHTER && getDistance(self->x, self->y, e->x, e->y) <= TRIGGER_RANGE)
		{
			self->systemPower--;
		
			if (self->systemPower <= 0)
			{
				self->health = 0;
			}
			
			return;
		}
	}
	
	self->systemPower = SYSTEM_POWER;
}


static void die(void)
{
	if (self->killedBy == player)
	{
		battle.stats[STAT_MINES_DESTROYED]++;
	}
	
	addMineExplosion();
	
	doSplashDamage();
	
	playBattleSound(SND_EXPLOSION_5, self->x, self->y);
	
	self->alive = ALIVE_DEAD;
}

static void doSplashDamage(void)
{
	Entity *e, **candidates;
	int i, dist;
	float damage, percent;

	candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->health > 0 && (e->type == ET_FIGHTER || e->type == ET_MINE) && !(e->flags & EF_IMMORTAL))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			
			if (dist <= DAMAGE_RANGE)
			{
				percent = dist;
				percent /= DAMAGE_RANGE;
				percent = 1 - percent;
				
				damage = 100;
				damage *= percent;
				
				if (e->type == ET_FIGHTER)
				{
					damageFighter(e, damage, 0);
				}
				else if (e->type == ET_MINE)
				{
					e->dx = e->x - self->x;
					e->dy = e->y - self->y;
					
					e->dx *= 0.01;
					e->dy *= 0.01;
				}
			}
		}
	}
}
