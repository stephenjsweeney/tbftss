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

#include "ai.h"

static void faceTarget(Entity *e);
static int isInFOV(Entity *e, int fov);
static void preAttack(void);
static void huntTarget(void);
static void huntAndAttackTarget(void);
static void moveToTargetLocation(void);
static void nextAction(void);
static void findTarget(void);
static int hasClearShot(void);
static void fallback(void);
static void moveToPlayer(void);
static int canAttack(Entity *e);
static int selectWeapon(int type);
static int nearExtractionPoint(void);
static void moveToExtractionPoint(void);
static int nearEnemies(void);
static int nearItems(void);
static void moveToItem(void);
static int nearTowableCraft(void);
static void moveToTowableCraft(void);
static int lookForPlayer(void);
static int lookForLeader(void);
static void fleeEnemies(void);
static int isRetreating(void);
static int getActionChance(int type);
static void doFighterAI(void);
static void doGunAI(void);
static void moveToLeader(void);
static void wander(void);
static void doWander(void);

void doAI(void)
{
	if ((self->aiFlags & (AIF_AVOIDS_COMBAT | AIF_EVADE)) && nearEnemies())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_DEFENSIVE) && rand() % 25 && nearEnemies())
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
	
	if (self->aiFlags & AIF_TOWS)
	{
		if (!self->towing && nearTowableCraft())
		{
			return;
		}
	}
	
	if ((self->aiFlags & AIF_RETREATS) && (battle.stats[STAT_TIME] % 60 == 0) && isRetreating())
	{
		return;
	}
	
	if (!(self->aiFlags & AIF_AVOIDS_COMBAT))
	{
		if (self->speed)
		{
			doFighterAI();
		}
		else
		{
			doGunAI();
		}
		
		return;
	}
	
	if (self->aiFlags & AIF_MOVES_TO_LEADER && lookForLeader())
	{
		return;
	}
	
	if ((self->aiFlags & (AIF_FOLLOWS_PLAYER|AIF_MOVES_TO_PLAYER)) && lookForPlayer())
	{
		return;
	}
	
	if (self->aiFlags & AIF_WANDERS)
	{
		doWander();
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
		
		if (!self->target)
		{
			/* takes priority over move to player */
			if (self->aiFlags & AIF_MOVES_TO_LEADER)
			{
				if (!lookForLeader())
				{
					if (self->aiFlags & AIF_MOVES_TO_PLAYER && player != NULL)
					{
						moveToPlayer();
					}
					else
					{
						applyFighterBrakes();
					}
				}
			}
			else if (self->aiFlags & AIF_MOVES_TO_PLAYER && player != NULL)
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
		self->targetLocation.x = self->target->x + (rand() % 250 - rand() % 250);
		self->targetLocation.y = self->target->y + (rand() % 250 - rand() % 250);
		self->action = moveToTargetLocation;
		self->aiActionTime = FPS;
	}
	else if (r <= getActionChance(AI_FALLBACK))
	{
		self->action = fallback;
		self->aiActionTime = FPS * 2;
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

static void doGunAI(void)
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
		
		if (!self->target)
		{
			return;
		}
	}
	
	r = rand() % 100;
	
	if (r <= 50)
	{
		self->action = huntTarget;
		self->aiActionTime = FPS * 3;
	}
	else
	{
		self->action = huntAndAttackTarget;
		self->aiActionTime = FPS;
	}
}

static int getActionChance(int type)
{
	switch (type)
	{
		case AI_EVADE:
			return 25 - (self->aiAggression * 4);
		
		case AI_FALLBACK:
			return 55 - (self->aiAggression * 4);
		
		case AI_HUNT:
			return 85 - (self->aiAggression * 4);
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
	int range = self->aiFlags & AIF_LONG_RANGE_FIRE ? (SCREEN_WIDTH * 1.5) : SCREEN_HEIGHT;
	
	faceTarget(self->target);
	
	if (dist <= range && hasClearShot())
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
	
	dist = closest = (battle.epic || (self->aiFlags & AIF_UNLIMITED_RANGE)) ? MAX_TARGET_RANGE : 2000;
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (dist / 2), self->y - (self->h / 2) - (dist / 2), self->w + dist, self->h + dist, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->active && (e->flags & EF_TAKES_DAMAGE) && (!(e->flags & EF_DISABLED)) && e->side != self->side && e->health > 0 && canAttack(e))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			
			if (dist < closest)
			{
				self->target = e;
				closest = dist;
			}
		}
	}
}

static int canAttack(Entity *e)
{
	self->selectedGunType = self->guns[0].type;
	
	if (e->aiFlags & (AIF_AVOIDS_COMBAT | AIF_EVADE) || e->flags & EF_SECONDARY_TARGET)
	{
		if (rand() % 10)
		{
			return 0;
		}
	}
	
	if (e->flags & EF_MUST_DISABLE)
	{
		if (e->systemPower > 0)
		{
			return selectWeapon(BT_MAG);
		}
		
		return 0;
	}
	
	if (e->flags & EF_NO_KILL)
	{
		return selectWeapon(BT_LASER) || selectWeapon(BT_MAG);
	}
	
	if (e->shield > 0)
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

static void faceTarget(Entity *e)
{
	int dir;
	int wantedAngle = getAngle(self->x, self->y, e->x, e->y);
	
	wantedAngle %= 360;
	
	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;
	
		self->angle += dir * TURN_SPEED;
		
		self->angle = mod(self->angle, 360);
		
		applyFighterBrakes();
	}
}

static int isInFOV(Entity *e, int fov)
{
	int angle, a, b;

	a = mod(self->angle - fov, 360);
	b = mod(self->angle + fov, 360);
	angle = getAngle(self->x, self->y, e->x, e->y);
	
	return (a < b) ? (a <= angle && angle <= b) : (a <= angle || angle <= b);
}

static int hasClearShot(void)
{
	int dist;
	Entity *e;
	
	if (isInFOV(self->target, 4))
	{
		dist = getDistance(self->x, self->y, self->target->x, self->target->y);
		
		for (e = battle.entityHead.next ; e != NULL ; e = e->next)
		{
			if (self->owner != NULL && self->owner == e->owner)
			{
				continue;
			}
			
			if (e->active && e != self && e != self->owner && e != self->target && (getDistance(self->x, self->y, e->x, e->y) < dist))
			{
				if (isInFOV(e, 8))
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
		if (!(self->aiFlags & AIF_MISSILE_BOAT))
		{
			/* force weapon selection, otherwise we'll keep using lasers / mag */
			canAttack(self->target);
			
			if (self->guns[0].type && (self->missiles == 0 || rand() % 50 > 0))
			{
				if (!dev.noAIWeapons)
				{
					fireGuns(self);
				}
			}
			else if (self->missiles && getDistance(self->x, self->y, self->target->x, self->target->y) >= 350)
			{
				if (!dev.noAIWeapons)
				{
					fireMissile(self);
				}
				
				self->action = doAI;
			}
		}
		else
		{
			fireRocket(self);
			
			/* don't constantly fire rockets like normal guns */
			if (rand() % 3)
			{
				self->action = doAI;
			}
		}
	}
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

static void moveToTargetLocation(void)
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
	float chance;
	
	if (!(self->flags & EF_RETREATING))
	{
		if (battle.numInitialEnemies > 0)
		{
			chance = battle.numEnemies;
			chance /= battle.numInitialEnemies;
			chance *= 256;
			
			if (battle.numEnemies > 0 && rand() % 100 > chance)
			{
				self->flags |= EF_RETREATING;
				
				self->aiFlags |= AIF_AVOIDS_COMBAT;
				self->aiFlags |= AIF_UNLIMITED_RANGE;
				self->aiFlags |= AIF_GOAL_EXTRACTION;
				
				addHudMessage(colors.red, "%s is retreating!", self->name);
				
				return 1;
			}
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
		if ((e->flags & EF_TAKES_DAMAGE) && e->side != self->side && !(e->flags & EF_DISABLED))
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
		
		/* dodge slightly */
		self->targetLocation.x += (rand() % 100 - rand() % 100);
		self->targetLocation.y += (rand() % 100 - rand() % 100);
		
		self->action = fleeEnemies;
		self->aiActionTime = FPS * 2;
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
			self->aiActionTime = (!self->towing) ? FPS / 2 : FPS * 2;
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
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (SCREEN_WIDTH / 4), self->y - (self->h / 2) - (SCREEN_HEIGHT / 4), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, self);
	
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
	
	candidates = getAllEntsWithin(self->x - (self->w / 2) - (SCREEN_WIDTH / 4), self->y - (self->h / 2) - (SCREEN_HEIGHT / 4), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, self);
	
	closest = MAX_TARGET_RANGE;
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if ((e->flags & (EF_DISABLED|EF_MISSION_TARGET)) == (EF_DISABLED|EF_MISSION_TARGET))
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
	
	nextAction();
}

static int lookForPlayer(void)
{
	int range = (self->aiFlags & AIF_MOVES_TO_PLAYER) ? MAX_TARGET_RANGE : 2000;
	
	if (player != NULL && getDistance(self->x, self->y, player->x, player->y) < range)
	{
		moveToPlayer();
		return 1;
	}
	
	return 0;
}

static int lookForLeader(void)
{
	long closest, distance;
	Entity *e;
	
	self->leader = NULL;
	closest = 0;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->flags & EF_AI_LEADER && e->side == self->side)
		{
			distance = getDistance(self->x, self->y, e->x, e->y);
			
			if (!closest || distance < closest)
			{
				self->leader = e;
				closest = distance;
			}
		}
	}
	
	if (self->leader)
	{
		self->aiActionTime = FPS;
		self->action = moveToLeader;
		return 1;
	}
	
	return 0;
}

static void moveToLeader(void)
{
	int dist = getDistance(self->x, self->y, self->leader->x, self->leader->y);
	
	if (dist <= 250)
	{
		applyFighterBrakes();
	}
	else
	{
		faceTarget(self->leader);
		
		applyFighterThrust();
	}
	
	nextAction();
}

static void doWander(void)
{
	self->targetLocation.x = 500 + (rand() % (BATTLE_AREA_WIDTH - 1000));
	self->targetLocation.y = 500 + (rand() % (BATTLE_AREA_HEIGHT - 1000));
	
	self->aiActionTime = FPS * 15;
	
	self->action = wander;
}

static void wander(void)
{
	moveToTargetLocation();
	
	if (nearEnemies())
	{
		self->aiActionTime = 0;
	}
	
	nextAction();
}
