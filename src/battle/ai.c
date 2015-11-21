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

static void faceTarget(Entity *f);
static int isInFOV(Entity *f, int fov);
static void preAttack(void);
static void huntTarget(void);
static void huntAndAttackTarget(void);
static void flyStraight(void);
static void evade(void);
static void nextAction(void);
static void findTarget(void);
static int hasClearShot(void);
static void boost(void);
static void fallback(void);
static void moveToPlayer(void);
static int canAttack(Entity *f);
static int selectWeapon(int type);
static int nearExtractionPoint(void);
static void moveToExtractionPoint(void);
static int nearEnemies(void);
static int nearItems(void);
static void moveToItem(void);
static int nearTowableCraft(void);
static void moveToTowableCraft(void);
static void lookForPlayer(void);
static void fleeEnemies(void);
static int isRetreating(void);
static int getActionChance(int type);
static void doFighterAI(void);

void doAI(void)
{
	if ((self->aiFlags & AIF_AVOIDS_COMBAT) && nearEnemies())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_DEFENSIVE) && rand() % 50 && nearEnemies())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_GOAL_EXTRACTION) && nearExtractionPoint())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_COLLECTS_ITEMS) && nearItems())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_TOWS) && nearTowableCraft())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_RETREATS) && (battle.stats[STAT_TIME] % 60 == 0) && isRetreating())
	{
		return;
	}
	
	if (!(self->aiFlags & AIF_AVOIDS_COMBAT))
	{
		doFighterAI();
		return;
	}
	
	if (self->aiFlags & AIF_FOLLOWS_PLAYER)
	{
		lookForPlayer();
		return;
	}
	
	/* no idea - just stay where you are */
	applyFighterBrakes();
}

static void doFighterAI(void)
{
	int r;
	
	/* don't hold a grudge against current target */
	if ((self->target != NULL && self->target->health <= 0) || rand() % 5 == 0)
	{
		self->action = doAI;
		self->target = NULL;
	}

	if (!self->target || self->target->systemPower <= 0)
	{
		findTarget();
		
		if (self->target == NULL)
		{
			if (self->aiFlags & AIF_FOLLOWS_PLAYER)
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
	
	/* don't start dodging, etc., if you're far from your target */
	if (getDistance(self->x, self->y, self->target->x, self->target->y) > SCREEN_WIDTH)
	{
		self->action = huntTarget;
		self->aiActionTime = FPS * 2;
		return;
	}
	
	r = rand() % 100;
	
	if (r <= getActionChance(AI_EVADE))
	{
		self->targetLocation.x = self->target->x + (rand() % 150 - rand() % 150);
		self->targetLocation.y = self->target->y + (rand() % 150 - rand() % 150);
		self->action = evade;
		self->aiActionTime = FPS;
	}
	else if (r <= getActionChance(AI_BOOST))
	{
		applyFighterThrust();
		self->action = boost;
		self->aiActionTime = FPS * 0.5;
	}
	else if (r <= getActionChance(AI_FALLBACK))
	{
		self->action = fallback;
		self->aiActionTime = FPS * 2;
	}
	else if (r <= getActionChance(AI_STRAIGHT))
	{
		self->action = flyStraight;
		self->aiActionTime = FPS;
	}
	else if (r <= getActionChance(AI_HUNT))
	{
		self->action = huntTarget;
		self->aiActionTime = FPS * 2;
	}
	else
	{
		self->action = huntAndAttackTarget;
		self->aiActionTime = FPS * 3;
	}
}

static int getActionChance(int type)
{
	switch (type)
	{
		case AI_EVADE:
			return 25 - (self->aiAggression * 3);
		
		case AI_BOOST:
			return 40 - (self->aiAggression * 4);
		
		case AI_FALLBACK:
			return 55 - (self->aiAggression * 5);
		
		case AI_STRAIGHT:
			return 70 - (self->aiAggression * 6);
		
		case AI_HUNT:
			return 85 - (self->aiAggression * 7);
	}
	
	return 100;
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
	
	dist = closest = (!battle.epic || (!(self->aiFlags & AIF_UNLIMITED_RANGE))) ? 2000 : MAX_TARGET_RANGE;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (dist / 2), self->y - (self->h / 2) - (dist / 2), self->w + dist, self->h + dist, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->active && e->type == ET_FIGHTER && e->side != self->side && e->health > 0 && canAttack(e))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			
			if (dist < closest)
			{
				if (self->target == NULL || ((self->target->aiFlags & AIF_AVOIDS_COMBAT) == 0) || ((self->target->aiFlags & AIF_AVOIDS_COMBAT) && rand() % 10) == 0)
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
	
	if (f->flags & EF_MUST_DISABLE)
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
	
	if (f->shield > 0)
	{
		selectWeapon(BT_LASER);
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
	
	if (self->action == doAI)
	{
		applyFighterThrust();
	}
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
		/* force weapon selection, otherwise we'll keep using lasers / mag */
		canAttack(self->target);
		
		if (self->guns[0].type && (self->missiles.ammo == 0 || rand() % 50 > 0))
		{
			fireGuns(self);
		}
		else if (self->missiles.ammo)
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

static void turnAndFly(int wantedAngle)
{
	int dir;
	
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

static void evade(void)
{
	int wantedAngle = getAngle(self->x, self->y, self->targetLocation.x, self->targetLocation.y);
	
	turnAndFly(wantedAngle);
}

static void fallback(void)
{
	int wantedAngle = 180 + getAngle(self->x, self->y, self->target->x, self->target->y);
	
	turnAndFly(wantedAngle);
}

static void nextAction(void)
{
	if (--self->aiActionTime <= 0)
	{
		self->action = doAI;
	}
}

static int isRetreating(void)
{
	if (!(self->flags & EF_RETREATING))
	{
		if (battle.numEnemies > 0 && rand() % (battle.numEnemies * 5) == 0)
		{
			self->flags |= EF_RETREATING;
			
			self->aiFlags |= AIF_AVOIDS_COMBAT;
			self->aiFlags |= AIF_UNLIMITED_RANGE;
			self->aiFlags |= AIF_GOAL_EXTRACTION;
			
			addHudMessage(colors.red, "%s is retreating!", self->name);
			
			return 1;
		}
	}
	
	return self->flags & EF_RETREATING;
}

static int nearEnemies(void)
{
	int i, numEnemies;
	Entity *e, **candidates;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - 500, self->y - (self->h / 2) - 500, 1000, 1000, self);
	
	self->target = NULL;
	self->targetLocation.x = self->targetLocation.y = 0;
	
	numEnemies = 0;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_FIGHTER && e->side != self->side)
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
	int wantedAngle = 180 + getAngle(self->x, self->y, self->targetLocation.x, self->targetLocation.y);
	
	turnAndFly(wantedAngle);
	
	nextAction();
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

static int nearExtractionPoint(void)
{
	int dist;
	
	self->target = NULL;
	
	if (battle.extractionPoint)
	{
		dist = getDistance(self->x, self->y, battle.extractionPoint->x, battle.extractionPoint->y);
		
		if (dist <= 2000 || self->aiFlags & AIF_UNLIMITED_RANGE)
		{
			self->target = battle.extractionPoint;
			self->action = moveToExtractionPoint;
			self->aiActionTime = FPS / 2;
		}
	}
	
	return self->target != NULL;
}

static void moveToExtractionPoint(void)
{
	faceTarget(self->target);
		
	applyFighterThrust();
	
	nextAction();
}

static int nearItems(void)
{
	int i;
	long closest, distance;
	Entity *e, **candidates;
	
	closest = MAX_TARGET_RANGE;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (GRID_CELL_WIDTH / 2), self->y - (self->h / 2) - (GRID_CELL_HEIGHT / 2), GRID_CELL_WIDTH, GRID_CELL_HEIGHT, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_ITEM)
		{
			distance = getDistance(self->x, self->y, e->x, e->y);
			
			if (distance < closest)
			{
				self->target = e;
				closest = distance;
			}
		}
	}
	
	if (self->target != NULL)
	{
		self->action = moveToItem;
		self->aiActionTime = FPS / 2;
	}
	
	return self->target != NULL;
}

static void moveToItem(void)
{
	if (self->target->alive == ALIVE_ALIVE)
	{
		faceTarget(self->target);		
		applyFighterThrust();
		return;
	}
	
	self->target = NULL;
	self->action = doAI;
}

static int nearTowableCraft(void)
{
	int i;
	long closest, distance;
	Entity *e, **candidates;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (GRID_CELL_WIDTH / 2), self->y - (self->h / 2) - (GRID_CELL_HEIGHT / 2), GRID_CELL_WIDTH, GRID_CELL_HEIGHT, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->type == ET_FIGHTER && (e->flags & EF_DISABLED))
		{
			distance = getDistance(self->x, self->y, e->x, e->y);
			
			if (distance < closest)
			{
				self->target = e;
				closest = distance;
			}
		}
	}
	
	if (self->target != NULL)
	{
		self->action = moveToTowableCraft;
		self->aiActionTime = FPS / 2;
	}
	
	return self->target != NULL;
}

static void moveToTowableCraft(void)
{
	faceTarget(self->target);
		
	applyFighterThrust();
}

static void lookForPlayer(void)
{
	long range = (self->aiFlags & AIF_UNLIMITED_RANGE) ? MAX_TARGET_RANGE : 1000;
	
	if (player != NULL && getDistance(self->x, self->y, player->x, player->y) < range)
	{
		moveToPlayer();
		return;
	}
	
	applyFighterBrakes();
}
