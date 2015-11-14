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
	{60, 65, 70, 75, 80},
	{50, 55, 60, 65, 70},
	{40, 45, 50, 55, 60},
	{30, 35, 40, 45, 50},
	{20, 25, 30, 35, 40}
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
static void moveToPlayer(void);
static int canAttack(Entity *f);
static int selectWeapon(int type);
static void flee(void);
static int nearExtractionPoint(void);
static int nearEnemies(void);
static void lookForPlayer(void);
static void fleeEnemies(void);
static void moveToExtractionPoint(void);

void doAI(void)
{
	int r;
	
	/* don't hold a grudge against current target */
	if ((self->target != NULL && self->target->health <= 0) || rand() % 2 == 0)
	{
		self->action = self->defaultAction;
		self->target = NULL;
	}

	if (!self->target || self->target->systemPower <= 0)
	{
		findTarget();
		
		if (self->target == NULL)
		{
			if (player != NULL && self->side == SIDE_ALLIES)
			{
				moveToPlayer();
			}
			else if (!(self->flags & EF_FLEEING))
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
		self->aiActionTime = FPS;
	}
	else if (r <= aggression[self->aggression][1])
	{
		self->action = boost;
		self->aiActionTime = FPS / 2;
	}
	else if (r <= aggression[self->aggression][2])
	{
		self->action = slow;
		self->aiActionTime = FPS / 2;
	}
	else if (r <= aggression[self->aggression][3])
	{
		self->action = flyStraight;
		self->aiActionTime = FPS;
	}
	else if (r <= aggression[self->aggression][4])
	{
		self->action = huntTarget;
		self->aiActionTime = FPS * 2;
	}
	else
	{
		self->action = huntAndAttackTarget;
		self->aiActionTime = FPS;
	}
	
	if (player != NULL && battle.numEnemies <= 2 && self->flags & EF_FLEES)
	{
		self->action = flee;
		self->aiActionTime = FPS * 3;
		if (!(self->flags & EF_FLEEING) && (self->flags & EF_MISSION_TARGET) && self->side != SIDE_ALLIES)
		{
			addHudMessage(colors.cyan, "Mission target is escaping!");
			self->flags |= EF_FLEEING;
		}
	}
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
	}
	else
	{
		applyFighterThrust();
	}
	
	nextAction();
}

static void findTarget(void)
{
	int i;
	Entity *e, **candidates;
	unsigned int dist, closest;
	
	dist = closest = (!battle.epic) ? 2000 : MAX_TARGET_RANGE;
	
	candidates = getAllEntsWithin(self->x - dist / 2, self->y - dist / 2, self->w + dist, self->h + dist, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->active && e->type == ET_FIGHTER && e->side != self->side && e->health > 0 && canAttack(e))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			
			if (dist < closest)
			{
				if (self->target == NULL || ((self->target->flags & EF_CIVILIAN) == 0) || ((self->target->flags & EF_CIVILIAN) && rand() % 10) == 0)
				{
					self->target = e;
					closest = dist;
				}
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
			if (f->active && f != self && f != self->target && (getDistance(self->x, self->y, f->x, f->y) < dist))
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
	if (!self->reload)
	{
		if (self->guns[0].type && (self->missiles.ammo == 0 || rand() % 50 > 0))
		{
			fireGuns(self);
		}
		else if (self->missiles.ammo && (self->target != player || rand() % 10 == 0))
		{
			fireMissile(self);
		}
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

static void flee(void)
{
	int dir;
	int wantedAngle = 180 + getAngle(self->x, self->y, player->x, player->y);
	
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

/* ====== Civilian AI ======= */

void doCivilianAI(void)
{
	if (!nearExtractionPoint() && !nearEnemies())
	{
		lookForPlayer();
	}
}

static int nearExtractionPoint(void)
{
	int i;
	Entity *e, **candidates;
	
	candidates = getAllEntsWithin(self->x - 500, self->y - 500, 1000, 1000, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_EXTRACTION_POINT)
		{
			self->target = e;
		}
	}
	
	if (self->target != NULL)
	{
		self->action = moveToExtractionPoint;
	}
	
	return self->target != NULL;
}

static void moveToExtractionPoint(void)
{
	faceTarget(self->target);
		
	applyFighterThrust();
}

static int nearEnemies(void)
{
	int i, numEnemies;
	Entity *e, **candidates;
	
	candidates = getAllEntsWithin(self->x - 1000, self->y - 1000, 2000, 2000, self);
	
	self->target = NULL;
	self->targetLocation.x = self->targetLocation.y = 0;
	
	numEnemies = 0;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_FIGHTER && e->side != SIDE_ALLIES)
		{
			self->targetLocation.x += e->x;
			self->targetLocation.y += e->y;
			numEnemies++;
		}
	}
	
	if (numEnemies)
	{
		self->targetLocation.x /= numEnemies;
		self->targetLocation.y /= numEnemies;
		self->action = fleeEnemies;
		self->aiActionTime = FPS / 2;
		return 1;
	}
	
	return 0;
}

static void fleeEnemies(void)
{
	int dir;
	int wantedAngle = 180 + getAngle(self->x, self->y, self->targetLocation.x, self->targetLocation.y);
	
	wantedAngle %= 360;
	
	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;
	
		self->angle += dir * TURN_SPEED;
		
		self->angle = mod(self->angle, 360);
	}
	
	applyFighterThrust();

	if (--self->aiActionTime <= 0)
	{
		self->action = doCivilianAI;
	}
}

static void lookForPlayer(void)
{
	if (player != NULL && getDistance(self->x, self->y, player->x, player->y) < 1000)
	{
		moveToPlayer();
		return;
	}
	
	applyFighterBrakes();
}
