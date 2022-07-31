/*
Copyright (C) 2015-2019,2022 Parallel Realities

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

#include "../common.h"

#include "../battle/effects.h"
#include "../battle/entities.h"
#include "../battle/fighters.h"
#include "../battle/objectives.h"
#include "../battle/quadtree.h"
#include "../battle/script.h"
#include "../game/trophies.h"
#include "../system/atlas.h"
#include "../system/sound.h"
#include "../system/util.h"
#include "mine.h"

#define DAMAGE_RANGE  250
#define SYSTEM_POWER  50
#define TRIGGER_RANGE 150

extern Battle  battle;
extern Entity *player;
extern Entity *self;

static void think(void);
static void die(void);
static void lookForFighters(void);
static void lookForPlayer(void);
static void doSplashDamage(void);

static AtlasImage *mineWarning = NULL;
static AtlasImage *mineNormal = NULL;
static AtlasImage *shadowMine = NULL;

Entity *spawnMine(int type)
{
	Entity *mine = spawnEntity();

	if (!mineWarning)
	{
		shadowMine = getAtlasImage("gfx/entities/shadowMine.png");
		mineNormal = getAtlasImage("gfx/entities/mine.png");
		mineWarning = getAtlasImage("gfx/entities/mineWarning.png");
	}

	STRNCPY(mine->name, "Mine", MAX_NAME_LENGTH);
	mine->type = type;
	mine->health = mine->maxHealth = 1;
	mine->speed = 1;
	mine->systemPower = SYSTEM_POWER;
	mine->texture = (type == ET_MINE) ? mineNormal : shadowMine;
	mine->action = think;
	mine->die = die;
	mine->flags = EF_TAKES_DAMAGE + EF_NO_PLAYER_TARGET + EF_SHORT_RADAR_RANGE + EF_NON_SOLID + EF_NO_HEALTH_BAR;

	if (type == ET_SHADOW_MINE)
	{
		mine->flags &= ~EF_NO_PLAYER_TARGET;
		mine->speed = 100 + rand() % 100;
		mine->speed *= 0.01;
	}

	mine->w = mine->texture->rect.w;
	mine->h = mine->texture->rect.h;

	return mine;
}

static void think(void)
{
	self->texture = (self->type == ET_MINE) ? mineNormal : shadowMine;

	self->angle += 0.1;

	if (self->angle >= 360)
	{
		self->angle -= 360;
	}

	self->dx *= 0.99;
	self->dy *= 0.99;

	if (self->type == ET_MINE)
	{
		lookForFighters();
	}
	else
	{
		lookForPlayer();
	}

	if (self->systemPower < SYSTEM_POWER && battle.stats[STAT_TIME] % 8 < 4)
	{
		playBattleSound(SND_MINE_WARNING, self->x, self->y);

		self->texture = mineWarning;
	}
}

static void lookForFighters(void)
{
	Entity *e, **candidates;
	int		i;

	candidates = getAllEntsInRadius(self->x, self->y, DAMAGE_RANGE, self);

	for (i = 0, e = candidates[i]; e != NULL; e = candidates[++i])
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

static void lookForPlayer(void)
{
	float dx, dy, norm;
	int	  distance;

	if (player->alive == ALIVE_ALIVE)
	{
		distance = getDistance(self->x, self->y, player->x, player->y);

		if (distance < SCREEN_WIDTH * 2)
		{
			dx = player->x - self->x;
			dy = player->y - self->y;
		}
		else
		{
			if (--self->aiActionTime > 0)
			{
				return;
			}

			dx = rand() % 1000;
			dx -= rand() % 1000;

			dy = rand() % 1000;
			dy -= rand() % 1000;

			self->aiActionTime = FPS * (5 + (rand() % 15));
		}

		norm = sqrt(dx * dx + dy * dy);

		dx /= norm;
		dy /= norm;

		self->dx = dx * self->speed;
		self->dy = dy * self->speed;

		if (distance <= TRIGGER_RANGE)
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

	updateObjective(self->name, TT_DESTROY);

	runScriptFunction("MINES_DESTROYED %d", battle.stats[STAT_MINES_DESTROYED]);
}

static void doSplashDamage(void)
{
	Entity *e, **candidates;
	int		i, dist, kills;
	float	damage, percent;

	candidates = getAllEntsInRadius(self->x, self->y, DAMAGE_RANGE, self);

	kills = 0;

	for (i = 0, e = candidates[i]; e != NULL; e = candidates[++i])
	{
		if (e->health > 0 && (e->type == ET_FIGHTER || e->type == ET_MINE) && !(e->flags & EF_IMMORTAL))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);

			if (dist <= DAMAGE_RANGE)
			{
				percent = dist;
				percent /= DAMAGE_RANGE;
				percent = 1 - percent;

				damage = DAMAGE_RANGE;
				damage *= percent;

				if (e->type == ET_FIGHTER)
				{
					damageFighter(e, damage, 0);

					if (self->killedBy == player && e != player && e->health <= 0)
					{
						kills++;
					}
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

	if (kills >= 2)
	{
		awardTrophy("2_BIRDS");
	}
}
