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
static int nearJumpgate(void);
static void moveToJumpgate(void);
static int nearEnemies(void);
static int nearItems(void);
static int nearMines(void);
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
static int selectWeaponForTarget(Entity *e);
static void deployMine(void);
static int isSurrendering(void);
static void doSurrender(void);
static void fleeWithinBattleArea(int x, int y, int numEnemies);
static int evadeNonKillTargets(void);

void doAI(void)
{
	if (self->aiFlags & AIF_DROPS_MINES)
	{
		deployMine();
	}
	
	if ((self->aiFlags & AIF_SURRENDERS) && (battle.stats[STAT_TIME] % 6 == 0) && isSurrendering())
	{
		return;
	}
	
	if ((self->aiFlags & (AIF_AVOIDS_COMBAT | AIF_EVADE)) && nearEnemies())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_DEFENSIVE) && rand() % 10 && nearEnemies())
	{
		return;
	}
	
	if (nearMines())
	{
		return;
	}
	
	if ((self->aiFlags & AIF_GOAL_JUMPGATE) && nearJumpgate())
	{
		/* near jumpgate, but you might decide to continue to fight, anyway */
		if ((self->aiFlags & AIF_COVERS_RETREAT) && rand() % 3)
		{
			return;
		}
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
	int r, breakOffChance;
	
	breakOffChance = (game.currentMission->challengeData.isChallenge || game.difficulty == DIFFICULTY_NORMAL) ? 5 : 1;
	
	/* don't hold a grudge against current target */
	if ((self->target != NULL && self->target->health <= 0) || rand() % breakOffChance == 0)
	{
		self->action = doAI;
		self->target = NULL;
	}

	if (!self->target || (self->target->systemPower <= 0 && (self->target->flags & EF_MUST_DISABLE)))
	{
		findTarget();
		
		if (!self->target)
		{
			if (!evadeNonKillTargets())
			{
				/* move to leader and wander take priority over move to player */
				if (self->aiFlags & AIF_MOVES_TO_LEADER)
				{
					if (!lookForLeader())
					{
						if (self->aiFlags & AIF_MOVES_TO_PLAYER && player->alive == ALIVE_ALIVE)
						{
							moveToPlayer();
						}
						else
						{
							applyFighterBrakes();
						}
					}
				}
				else if (self->aiFlags & AIF_WANDERS)
				{
					doWander();
				}
				else if (self->aiFlags & AIF_MOVES_TO_PLAYER && player->alive == ALIVE_ALIVE)
				{
					moveToPlayer();
				}
				else
				{
					applyFighterBrakes();
				}
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
	
	/* if your target is disabled, just shoot it..! */
	r = (self->target->flags & EF_DISABLED) ? 100 : rand() % 100;
	
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
	int r, breakOffChance;
	
	breakOffChance = (game.currentMission->challengeData.isChallenge || game.difficulty == DIFFICULTY_NORMAL) ? 5 : 1;
	
	/* don't hold a grudge against current target */
	if ((self->target != NULL && self->target->health <= 0) || rand() % breakOffChance == breakOffChance)
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
	
	if (!game.currentMission->challengeData.isChallenge && game.difficulty == DIFFICULTY_EASY)
	{
		r -= 35;
	}
	
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
	
	dist = closest = (battle.isEpic || (self->aiFlags & AIF_UNLIMITED_RANGE)) ? MAX_TARGET_RANGE : SCREEN_WIDTH;
	
	candidates = getAllEntsInRadius(self->x, self->y, dist, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (canAttack(e) && selectWeaponForTarget(e))
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
	if (!e->active || e->side == self->side || e->health <= 0 || (e->flags & EF_AI_IGNORE))
	{
		return 0;
	}
	
	if (!(e->flags & EF_TAKES_DAMAGE))
	{
		return 0;
	}
	
	if (!(self->aiFlags & AIF_ASSASSIN))
	{
		if (e->flags & EF_MUST_DISABLE)
		{
			return e->systemPower > 0;
		}
		
		if (!(e->flags & EF_AI_TARGET))
		{
			if (e->aiFlags & (AIF_AVOIDS_COMBAT | AIF_EVADE) || e->flags & EF_SECONDARY_TARGET)
			{
				return !(rand() % 5);
			}
		}
		
		/* low chance of attacking something else */
		if ((self->aiFlags & AIF_TARGET_FOCUS) && (!(e->flags & EF_AI_TARGET)))
		{
			return !(rand() % 100);
		}
	}
	
	return 1;
}

static int selectWeaponForTarget(Entity *e)
{
	self->selectedGunType = self->guns[0].type;
	
	/* if you're an assassin, just kill the target */
	if (!(self->aiFlags & AIF_ASSASSIN))
	{
		if (e->flags & EF_MUST_DISABLE)
		{
			return selectWeapon(BT_MAG);
		}
		
		if (e->flags & EF_NO_KILL)
		{
			if (!(e->flags & EF_DISABLED))
			{
				return selectWeapon(BT_LASER) || selectWeapon(BT_MAG);
			}
			
			return 0;
		}
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
	if (!self->reload && !dev.noAIWeapons)
	{
		if (!(self->aiFlags & AIF_MISSILE_BOAT))
		{
			/* force weapon selection, otherwise we'll keep using lasers / mag */
			selectWeaponForTarget(self->target);
			
			if (self->guns[0].type && (self->missiles == 0 || rand() % 50 > 0))
			{
				fireGuns(self);
			}
			else if (self->missiles && (!(self->target->flags & (EF_NO_KILL|EF_MUST_DISABLE))) && getDistance(self->x, self->y, self->target->x, self->target->y) >= 350)
			{
				fireMissile(self);
				
				self->action = doAI;
			}
		}
		else if (!(self->target->flags & EF_NO_KILL))
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

static void turnToFace(int wantedAngle)
{
	int dir;
	
	wantedAngle %= 360;
	
	if (fabs(wantedAngle - self->angle) > TURN_THRESHOLD)
	{
		dir = ((int)(wantedAngle - self->angle + 360)) % 360 > 180 ? -1 : 1;
	
		self->angle += dir * TURN_SPEED;
		
		self->angle = mod(self->angle, 360);
	}
}

static void turnAndFly(int wantedAngle)
{
	turnToFace(wantedAngle);
	
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
				self->aiFlags |= AIF_GOAL_JUMPGATE;
				
				addHudMessage(colors.red, _("%s is retreating!"), self->name);
				
				return 1;
			}
		}
	}
	
	return self->flags & EF_RETREATING;
}

static int isSurrendering(void)
{
	float chance;
	
	if (!(self->aiFlags & AIF_SURRENDERED))
	{
		if (self->health < self->maxHealth)
		{
			chance = self->health;
			chance /= self->maxHealth;
			chance *= 100;
			
			if (rand() % 100 > chance)
			{
				self->aiActionTime = FPS * 3;
				
				self->aiFlags |= AIF_AVOIDS_COMBAT;
				self->aiFlags |= AIF_SURRENDERING;
				self->aiFlags &= ~AIF_SURRENDERS;
				
				self->flags |= EF_MUST_DISABLE;
				self->flags |= EF_MISSION_TARGET;
				
				nearEnemies();
				
				self->action = doSurrender;
				
				battle.stats[STAT_ENEMIES_SURRENDERED]++;
				
				runScriptFunction("ENEMIES_SURRENDERED %d", battle.stats[STAT_ENEMIES_SURRENDERED]);
				
				addHudMessage(colors.white, _("%s has surrendered"), self->name);
				
				return 1;
			}
		}
	}
	
	return 0;
}

static void doSurrender(void)
{
	if (--self->aiActionTime <= 0)
	{
		self->aiFlags &= ~AIF_SURRENDERING;
		self->aiFlags |= AIF_SURRENDERED;
		self->speed = 1.5;
		
		nextAction();
	}
}

static int nearEnemies(void)
{
	int i, numEnemies, x, y;
	Entity *e, **candidates;
	
	candidates = getAllEntsInRadius(self->x, self->y, SCREEN_WIDTH, self);
	
	self->target = NULL;
	x = y = 0;
	
	numEnemies = 0;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if ((e->flags & EF_TAKES_DAMAGE) && e->side != SIDE_NONE && e->side != self->side && !(e->flags & EF_DISABLED))
		{
			if ((self->aiFlags & AIF_TARGET_FOCUS) && (e->flags & EF_AI_TARGET))
			{
				continue;
			}
			
			if (getDistance(e->x, e->y, self->x, self->y) <= SCREEN_WIDTH)
			{
				x += e->x;
				y += e->y;
				numEnemies++;
			}
		}
	}
	
	if (numEnemies)
	{
		fleeWithinBattleArea(x, y, numEnemies);
		
		return 1;
	}
	
	return 0;
}

static int evadeNonKillTargets(void)
{
	int i, numEnemies, x, y;
	Entity *e, **candidates;
	
	candidates = getAllEntsInRadius(self->x, self->y, SCREEN_WIDTH, self);
	
	self->target = NULL;
	x = y = 0;
	
	numEnemies = 0;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if ((e->flags & EF_TAKES_DAMAGE) && e->side != SIDE_NONE && e->side != self->side && (!(e->flags & EF_DISABLED)))
		{
			if (getDistance(e->x, e->y, self->x, self->y) <= SCREEN_WIDTH)
			{
				x += e->x;
				y += e->y;
				numEnemies++;
			}
		}
	}
	
	if (numEnemies)
	{
		fleeWithinBattleArea(x, y, numEnemies);
		
		return 1;
	}
	
	return 0;
}

static void fleeWithinBattleArea(int x, int y, int numEnemies)
{
	self->targetLocation.x = x;
	self->targetLocation.y = y;
	
	self->targetLocation.x /= numEnemies;
	self->targetLocation.y /= numEnemies;
	
	/* dodge slightly */
	self->targetLocation.x += (rand() % 100 - rand() % 100);
	self->targetLocation.y += (rand() % 100 - rand() % 100);
	
	self->aiActionTime = FPS * 2;
	
	if (self->targetLocation.x < SCREEN_WIDTH)
	{
		self->targetLocation.x = -SCREEN_WIDTH;
	}
	
	if (self->targetLocation.x >= BATTLE_AREA_WIDTH - SCREEN_WIDTH)
	{
		self->targetLocation.x = BATTLE_AREA_WIDTH;
	}
	
	if (self->targetLocation.y < SCREEN_HEIGHT)
	{
		self->targetLocation.y = -SCREEN_HEIGHT;
	}
	
	if (self->targetLocation.y >= BATTLE_AREA_HEIGHT - SCREEN_HEIGHT)
	{
		self->targetLocation.y = BATTLE_AREA_HEIGHT;
	}
	
	self->action = fleeEnemies;
}

static void deployMine(void)
{
	Entity *mine;
	
	if (!self->reload && self->thrust > 0)
	{
		mine = spawnMine(ET_MINE);
		mine->x = self->x;
		mine->y = self->y;
		mine->dx = rand() % 20 - rand() % 20;
		mine->dx *= 0.1;
		mine->dy = rand() % 20 - rand() % 20;
		mine->dy *= 0.1;
		mine->side = self->side;
		
		self->reload = rand() % (FPS * 3);
	}
}

static int nearMines(void)
{
	int i, numMines, x, y;
	Entity *e, **candidates;
	
	candidates = getAllEntsInRadius(self->x, self->y, SCREEN_HEIGHT, self);
	
	x = y = 0;
	
	numMines = 0;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->side != self->side && e->type == ET_MINE && getDistance(e->x, e->y, self->x, self->y) <= SCREEN_HEIGHT)
		{
			x += e->x;
			y += e->y;
			numMines++;
		}
	}
	
	if (numMines)
	{
		fleeWithinBattleArea(x, y, numMines);
		
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
	int wantedAngle;
	int dist = getDistance(self->x, self->y, player->x, player->y);
	
	if (dist <= 250)
	{
		if (player->thrust > 0.1)
		{
			wantedAngle = getAngle(player->x, player->y, player->x + (player->dx * 1000), player->y + (player->dy * 1000));
		
			turnToFace(wantedAngle);
			
			if (self->thrust > player->thrust)
			{
				applyFighterBrakes();
			}
			else
			{
				applyFighterThrust();
			}
		}
		else
		{
			applyFighterBrakes();
		}
	}
	else
	{
		faceTarget(player);
		
		applyFighterThrust();
	}
}

static int nearJumpgate(void)
{
	int dist;
	
	self->target = NULL;
	
	if (battle.jumpgate)
	{
		dist = getDistance(self->x, self->y, battle.jumpgate->x, battle.jumpgate->y);
		
		if (dist <= 2000 || self->aiFlags & AIF_UNLIMITED_RANGE)
		{
			self->target = battle.jumpgate;
			self->action = moveToJumpgate;
			self->aiActionTime = (!self->towing) ? FPS / 2 : FPS * 2;
		}
	}
	
	return self->target != NULL;
}

static void moveToJumpgate(void)
{
	self->target = battle.jumpgate;
	
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
	
	candidates = getAllEntsInRadius(self->x, self->y, SCREEN_WIDTH / 2, self);
	
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
	long closest, dist;
	Entity *e, **candidates;
	
	dist = closest = (battle.isEpic || (self->aiFlags & AIF_UNLIMITED_RANGE)) ? MAX_TARGET_RANGE : 2000;
	
	candidates = getAllEntsInRadius(self->x, self->y, dist, self);
	
	self->target = NULL;
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (!e->owner && (e->flags & (EF_DISABLED|EF_MISSION_TARGET)) == (EF_DISABLED|EF_MISSION_TARGET) && (e->flags & EF_ROPED_ATTACHED) == 0)
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			
			if (dist < closest)
			{
				self->target = e;
				closest = dist;
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
	
	if (player->alive == ALIVE_ALIVE && getDistance(self->x, self->y, player->x, player->y) < range)
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
		self->aiActionTime = FPS + (rand() % FPS);
		return 1;
	}
	
	return 0;
}

static void moveToLeader(void)
{
	int wantedAngle;
	int dist = getDistance(self->x, self->y, self->leader->x, self->leader->y);
	
	if (dist <= ((self->leader->type != ET_CAPITAL_SHIP) ? 350 : 550))
	{
		if (self->leader->thrust > 0.1)
		{
			wantedAngle = getAngle(self->leader->x, self->leader->y, self->leader->x + (self->leader->dx * 1000), self->leader->y + (self->leader->dy * 1000));
			
			turnToFace(wantedAngle);
			
			if (self->thrust > self->leader->thrust)
			{
				applyFighterBrakes();
			}
			else
			{
				applyFighterThrust();
			}
		}
		else
		{
			applyFighterBrakes();
		}
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
	
	if (nearEnemies() || getDistance(self->x, self->y, self->targetLocation.x, self->targetLocation.y) <= 100)
	{
		self->aiActionTime = 0;
	}
}

/*
 * Used only for the optional missions, in Pandoran-controlled space. We can therefore hardcode the response.
 */
void checkSuspicionLevel(void)
{
	long distance;
	
	if (battle.status == MS_IN_PROGRESS && player->side != SIDE_ALLIES)
	{
		battle.suspicionCoolOff = MAX(battle.suspicionCoolOff - 1, 0);
		
		distance = getDistance(self->x, self->y, player->x, player->y);
		
		/* raise if player is too far away and there are no enemies */
		if (battle.suspicionCoolOff == 0 && battle.numEnemies == 0 && distance > SCREEN_HEIGHT)
		{
			battle.suspicionLevel++;
		}
		
		/* raise if there are enemies around, lower if none and player is close to leader */
		if (battle.stats[STAT_TIME] % 5 == 0)
		{
			if (battle.numEnemies > 0)
			{
				battle.suspicionLevel++;
				battle.suspicionCoolOff = FPS * 30;
				
				if (game.difficulty == DIFFICULTY_EASY)
				{
					battle.suspicionCoolOff = FPS * 60;
				}
			}
			else if (distance <= SCREEN_HEIGHT / 2)
			{
				if (game.difficulty == DIFFICULTY_EASY)
				{
					battle.suspicionLevel--;
				}
				
				battle.suspicionLevel = MAX(battle.suspicionLevel - 1, 0);
			}
		}
	}
}

/* only used in final optional mission */
void checkZackariaSuspicionLevel(void)
{
	if (battle.zackariaSuspicionLevel < MAX_ZAK_SUSPICION_LEVEL)
	{
		if (getDistance(self->x, self->y, player->x, player->y) < SCREEN_HEIGHT)
		{
			if (++battle.zackariaSuspicionLevel >= MAX_ZAK_SUSPICION_LEVEL)
			{
				runScriptFunction("Zackaria");
				
				battle.unwinnable = 1;
				
				battle.hasSuspicionLevel = 0;
				
				self->thrust = self->speed = 0;
			}
		}
	}
}
