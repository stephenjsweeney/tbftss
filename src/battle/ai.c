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

#include "ai.h"

static int aggression[][5] =
{
	{25, 35, 35, 40, 50},
	{20, 30, 30, 35, 40},
	{15, 20, 25, 30, 35},
	{10, 15, 20, 25, 30},
	{5, 10, 15, 20, 25}
};

static void faceTarget(Entity *f);
static int isInFOV(Entity *f, int fov);
static void preAttack(void);
static void huntTarget(void);
static void huntAndAttackTarget(void);
static void flyStraight(void);
static void dodge(void);
static void nextAction(void);
static void findTarget(void);
static int hasClearShot(void);
static void boost(void);
static void slow(void);
static int targetOutOfRange(void);
static void moveToPlayer(void);
static int canAttack(Entity *f);
static int selectWeapon(int type);

void doAI(void)
{
	int r;
	
	if (!self->target || targetOutOfRange() || self->target->systemPower <= 0)
	{
		findTarget();
		
		if (self->target == NULL)
		{
			if (player != NULL && self->side == player->side)
			{
				moveToPlayer();
			}
			else
			{
				applyFighterBrakes();
			}
			return;
		}
	}
	
	r = rand() % 100;
	
	if (r <= aggression[self->aggression][0])
	{
		self->action = dodge;
		self->aiActionTime = FPS * 2;
	}
	else if (r <= aggression[self->aggression][1])
	{
		self->action = boost;
		self->aiActionTime = FPS * 1;
	}
	else if (r <= aggression[self->aggression][2])
	{
		self->action = slow;
		self->aiActionTime = FPS * 1;
	}
	else if (r <= aggression[self->aggression][3])
	{
		self->action = flyStraight;
		self->aiActionTime = FPS * 1;
	}
	else if (r <= aggression[self->aggression][4])
	{
		self->action = huntTarget;
		self->aiActionTime = FPS * 2;
	}
	else
	{
		self->action = huntAndAttackTarget;
		self->aiActionTime = FPS * 1;
	}
}

static int targetOutOfRange(void)
{
	return getDistance(self->x, self->y, self->target->x, self->target->y) > 2000;
}

static void huntTarget(void)
{
	faceTarget(self->target);
	
	applyFighterThrust();
	
	nextAction();
}

static void huntAndAttackTarget(void)
{
	int dist = getDistance(self->x, self->y, self->target->x, self->target->y);
	
	faceTarget(self->target);
	
	if (dist <= 500 && hasClearShot())
	{
		preAttack();
	}
	
	if (dist <= 250)
	{
		applyFighterBrakes();
		self->aiActionTime = MIN(FPS, self->aiActionTime);
	}
	else
	{
		applyFighterThrust();
	}
	
	nextAction();
}

static void findTarget(void)
{
	Entity *f;
	int closest = 2000;
	int dist = 2000;
	
	self->target = NULL;
	
	for (f = battle.entityHead.next ; f != NULL ; f = f->next)
	{
		if (f->type == ET_FIGHTER && f->side != self->side && f->health > 0 && canAttack(f))
		{
			dist = getDistance(self->x, self->y, f->x, f->y);
			if (dist < closest)
			{
				self->target = f;
				closest = dist;
			}
		}
	}
}

static int canAttack(Entity *f)
{
	self->selectedGunType = self->guns[0].type;
	
	if (f->flags & EF_DISABLE)
	{
		if (f->systemPower > 0)
		{
			return selectWeapon(BT_MAG);
		}
		
		return 0;
	}
	
	if (f->flags & EF_NO_KILL)
	{
		return selectWeapon(BT_LASER) || selectWeapon(BT_MAG);
	}
	
	return 1;
}

static int selectWeapon(int type)
{
	int i;
	
	for (i = 0 ; i < MAX_FIGHTER_GUNS ; i++)
	{
		if (self->guns[i].type == type)
		{
			self->selectedGunType = type;
			return 1;
		}
	}
	
	return 0;
}

static void faceTarget(Entity *f)
{
	int dir;
	int wantedAngle = getAngle(self->x, self->y, f->x, f->y);
	
	wantedAngle %= 360;
	
	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;
	
		self->angle += dir * TURN_SPEED;
		
		self->angle = mod(self->angle, 360);
		
		applyFighterBrakes();
	}
}

static int isInFOV(Entity *f, int fov)
{
	int angle, a, b;

	a = mod(self->angle - fov, 360);
	b = mod(self->angle + fov, 360);
	angle = getAngle(self->x, self->y, f->x, f->y);
	
	return (a < b) ? (a <= angle && angle <= b) : (a <= angle || angle <= b);
}

static void boost(void)
{
	self->dx *= 1.001;
	self->dy *= 1.001;
	
	nextAction();
}

static void slow(void)
{
	self->dx *= 0.95;
	self->dy *= 0.95;
	
	nextAction();
}

static int hasClearShot(void)
{
	int dist;
	Entity *f;
	
	if (isInFOV(self->target, 4))
	{
		dist = getDistance(self->x, self->y, self->target->x, self->target->y);
		
		for (f = battle.entityHead.next ; f != NULL ; f = f->next)
		{
			if (f != self && f != self->target && (getDistance(self->x, self->y, f->x, f->y) < dist))
			{
				if (isInFOV(f, 8))
				{
					return 0;
				}
			}
		}
		
		return 1;
	}
	
	return 0;
}

static void preAttack(void)
{
	if (!self->reload && self->guns[0].type)
	{
		fireGuns(self);
	}
}

static void flyStraight(void)
{
	applyFighterThrust();
	
	nextAction();
}

static void dodge(void)
{
	int dir;
	int wantedAngle = 180 + getAngle(self->x, self->y, self->target->x, self->target->y);
	
	wantedAngle %= 360;
	
	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;
	
		self->angle += dir * TURN_SPEED;
		
		self->angle = mod(self->angle, 360);
	}
	
	applyFighterThrust();

	nextAction();
}

static void nextAction(void)
{
	if (--self->aiActionTime <= 0)
	{
		self->action = doAI;
	}
}

static void moveToPlayer(void)
{
	int dist = getDistance(self->x, self->y, player->x, player->y);
	
	if (dist <= 250)
	{
		applyFighterBrakes();
		
		self->aiActionTime = MIN(FPS, self->aiActionTime);
	}
	else
	{
		faceTarget(player);
		
		applyFighterThrust();
	}
}
