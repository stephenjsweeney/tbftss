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

#include "entities.h"

static void drawEntity(Entity *e);
static void doEntity(void);
static void alignComponents(void);
static void activateEpicFighters(int side);
static void restrictToBattleArea(Entity *e);
static void drawTargetRects(Entity *e);
static void drawHealthBar(Entity *e);
static int drawComparator(const void *a, const void *b);
static void notifyNewArrivals(void);
static int isComponent(Entity *e);

static Entity deadHead;
static Entity *deadTail;
static int disabledGlow;
static int disabledGlowDir;

void initEntities(void)
{
	memset(&deadHead, 0, sizeof(Entity));

	deadTail = &deadHead;

	disabledGlow = DISABLED_GLOW_MAX;
	disabledGlowDir = -DISABLED_GLOW_SPEED;
}

Entity *spawnEntity(void)
{
	Entity *e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	e->active = 1;

	battle.entityTail->next = e;
	battle.entityTail = e;

	return e;
}

void doEntities(void)
{
	int numAllies, numEnemies;
	int numActiveAllies, numActiveEnemies;
	int numSpawnedEnemies;
	Entity *e, *prev;

	prev = &battle.entityHead;

	battle.hasThreats = numAllies = numEnemies = numActiveAllies = numActiveEnemies = numSpawnedEnemies = 0;

	if (dev.playerImmortal)
	{
		player->health = player->maxHealth;
		player->shield = player->maxShield;
	}

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		removeFromQuadtree(e, &battle.quadtree);

		if (dev.allImmortal)
		{
			e->health = e->maxHealth;
			e->shield = e->maxShield;
		}
		
		if (e->active)
		{
			self = e;

			e->reload = MAX(e->reload - 1, 0);

			if (e->shieldRechargeRate)
			{
				if (e->shield >= 0)
				{
					if (--e->shieldRecharge <= 0)
					{
						e->shield = MIN(e->shield + 1, e->maxShield);

						e->shieldRecharge = e->shieldRechargeRate;
					}
				}
				else
				{
					e->shield++;
				}
			}

			e->armourHit = MAX(e->armourHit - 25, 0);
			e->shieldHit = MAX(e->shieldHit - 5, 0);
			e->systemHit = MAX(e->systemHit - 25, 0);

			e->aiDamageTimer = MAX(e->aiDamageTimer - 1, 0);
			if (!e->aiDamageTimer)
			{
				e->aiDamagePerSec = 0;
				e->aiFlags &= ~AIF_EVADE;
			}

			switch (e->type)
			{
				case ET_FIGHTER:
					doFighter();
					break;

				case ET_CAPITAL_SHIP:
					doCapitalShip();
					break;

				default:
					doEntity();
					break;
			}

			if (e->alive == ALIVE_ALIVE || e->alive == ALIVE_DYING)
			{
				if (e->action != NULL)
				{
					if (dev.noEntityActions)
					{
						e->thinkTime = 2;
					}

					if (--e->thinkTime <= 0)
					{
						e->thinkTime = 0;
						e->action();
					}
				}
				
				doRope(e);
				
				restrictToBattleArea(e);
				
				if (!e->speed)
				{
					e->dx = e->dy = 0;
				}
				
				e->x += e->dx;
				e->y += e->dy;
				
				addToQuadtree(e, &battle.quadtree);
			}
			else
			{
				if (e == battle.entityTail)
				{
					battle.entityTail = prev;
				}

				if (e == battle.missionTarget)
				{
					battle.missionTarget = NULL;
				}
				
				if (e->killedBy == player && battle.hasSuspicionLevel)
				{
					battle.suspicionLevel -= (MAX_SUSPICION_LEVEL * 0.12);
				}

				if (e == player)
				{
					battle.playerSelect = battle.isEpic;
				}

				cutRope(e);

				prev->next = e->next;
				
				e->next = NULL;
				
				deadTail->next = e;
				deadTail = e;
				
				/* actually just creates another fighter in this one's place */
				if (e->type == ET_FIGHTER && battle.isEpic && e->side != player->side && battle.unlimitedEnemies)
				{
					resetFighter(e);
				}

				e = prev;
			}
		}

		if (e->type == ET_FIGHTER || e->type == ET_CAPITAL_SHIP)
		{
			if (e->side == player->side)
			{
				numAllies++;

				if (e->health > 0 && e->active)
				{
					numActiveAllies++;
				}
			}
			else
			{
				numEnemies++;

				if (e->health > 0 && e->active)
				{
					numActiveEnemies++;
					
					if (e->spawned)
					{
						numSpawnedEnemies++;
					}
				}
				
				if (!(e->flags & EF_DISABLED))
				{
					battle.hasThreats = 1;
				}
			}
		}

		prev = e;
	}

	battle.numAllies = (battle.isEpic) ? numAllies : numActiveAllies;
	battle.numEnemies = (battle.isEpic) ? numEnemies : numActiveEnemies;
	
	if (battle.status == MS_IN_PROGRESS && battle.stats[STAT_TIME] % (FPS * 30) == 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "numEnemies=%d, numActiveEnemies=%d, hasThreats=%d", numEnemies, numActiveEnemies, battle.hasThreats);
	}

	if (battle.isEpic && battle.stats[STAT_TIME] % FPS == 0)
	{
		if (numActiveAllies < battle.epicFighterLimit)
		{
			activateEpicFighters(SIDE_ALLIES);
		}
		
		numActiveEnemies -= numSpawnedEnemies;

		if (numActiveEnemies < battle.epicFighterLimit)
		{
			activateEpicFighters(SIDE_NONE);
		}
	}

	alignComponents();

	disabledGlow = MAX(DISABLED_GLOW_MIN, MIN(disabledGlow + disabledGlowDir, DISABLED_GLOW_MAX));

	if (disabledGlow <= DISABLED_GLOW_MIN)
	{
		disabledGlowDir = DISABLED_GLOW_SPEED;
	}
	else if (disabledGlow >= DISABLED_GLOW_MAX)
	{
		disabledGlowDir = -DISABLED_GLOW_SPEED;
	}
}

static void restrictToBattleArea(Entity *e)
{
	float force;

	if (e->x <= BATTLE_AREA_EDGE)
	{
		force = BATTLE_AREA_EDGE - e->x;
		e->dx += force * 0.001;
		e->dx *= 0.95;
	}

	if (e->y <= BATTLE_AREA_EDGE)
	{
		force = BATTLE_AREA_EDGE - e->y;
		e->dy += force * 0.001;
		e->dy *= 0.95;
	}

	if (e->x >= BATTLE_AREA_WIDTH - BATTLE_AREA_EDGE)
	{
		force = e->x - (BATTLE_AREA_WIDTH - BATTLE_AREA_EDGE);
		e->dx -= force * 0.001;
		e->dx *= 0.95;
	}

	if (e->y >= BATTLE_AREA_HEIGHT - BATTLE_AREA_EDGE)
	{
		force = e->y - (BATTLE_AREA_HEIGHT - BATTLE_AREA_EDGE);
		e->dy -= force * 0.001;
		e->dy *= 0.95;
	}
}

static void doEntity(void)
{
	if (self->die)
	{
		if (self->health <= 0 && self->alive == ALIVE_ALIVE)
		{
			self->health = 0;
			self->alive = ALIVE_DYING;
			self->die();

			if (self == battle.missionTarget)
			{
				battle.missionTarget = NULL;
			}
		}
	}
	else
	{
		if (self->alive == ALIVE_DYING)
		{
			self->alive = ALIVE_DEAD;
		}
		else if (self->health <= 0)
		{
			self->alive = ALIVE_DYING;
		}
	}
}

static void alignComponents(void)
{
	Entity *e;
	float x, y;
	float c, s;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (isComponent(e))
		{
			removeFromQuadtree(e, &battle.quadtree);
			
			s = sin(TO_RAIDANS(e->owner->angle));
			c = cos(TO_RAIDANS(e->owner->angle));

			x = (e->offsetX * c) - (e->offsetY * s);
			y = (e->offsetX * s) + (e->offsetY * c);

			x += e->owner->x;
			y += e->owner->y;

			e->x = x;
			e->y = y;

			if (e->flags & EF_STATIC)
			{
				e->angle = e->owner->angle;
			}

			addToQuadtree(e, &battle.quadtree);
		}
	}
}

static int isComponent(Entity *e)
{
	return (e->type == ET_COMPONENT || e->type == ET_COMPONENT_GUN || e->type == ET_COMPONENT_ENGINE);
}

void drawEntities(void)
{
	int i;
	Entity *e, **candidates;
	
	candidates = getAllEntsWithin(battle.camera.x, battle.camera.y, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);
	
	/* counting entities to draw */
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i]) {};
	
	qsort(candidates, i, sizeof(Entity*), drawComparator);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		self = e;

		if (e->draw)
		{
			e->draw();
		}
		else
		{
			drawEntity(e);
		}
		
		drawHealthBar(e);
		
		drawTargetRects(e);
		
		drawRope(e);
	}
}

static void drawEntity(Entity *e)
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

	if (e->flags & EF_DISABLED)
	{
		SDL_SetTextureColorMod(e->texture, disabledGlow, disabledGlow, 255);
	}

	blitRotated(e->texture, e->x - battle.camera.x, e->y - battle.camera.y, e->angle);

	if (e->shieldHit > 0)
	{
		drawShieldHitEffect(e);
	}

	SDL_SetTextureColorMod(e->texture, 255, 255, 255);
}

static void drawHealthBar(Entity *e)
{
	SDL_Rect r;
	
	if (app.gameplay.healthBars && !(e->flags & EF_NO_HEALTH_BAR) && e->health > 0)
	{
		r.x = e->x - (e->w / 2) - battle.camera.x;
		r.y = e->y - e->h - battle.camera.y;
		r.w = 32;
		r.h = 1;
		
		if (e->side == player->side || e->flags & EF_FRIENDLY_HEALTH_BAR)
		{
			SDL_SetRenderDrawColor(app.renderer, 0, 128, 0, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(app.renderer, 128, 0, 0, 255);
		}
		
		SDL_RenderFillRect(app.renderer, &r);
		
		r.w = 32 * (e->health * 1.0f / e->maxHealth);
		
		if (e->side == player->side || e->flags & EF_FRIENDLY_HEALTH_BAR)
		{
			SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
		}
		
		SDL_RenderFillRect(app.renderer, &r);
	}
}

static void drawTargetRects(Entity *e)
{
	SDL_Rect r;

	int size = MAX(e->w, e->h) + 16;

	if (player->alive == ALIVE_ALIVE && e == player->target)
	{
		r.x = e->x - (size / 2) - battle.camera.x;
		r.y = e->y - (size / 2) - battle.camera.y;
		r.w = size;
		r.h = size;

		SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
		SDL_RenderDrawRect(app.renderer, &r);
	}

	if ((e == battle.missionTarget || e->flags & EF_MISSION_TARGET) && (e->flags & EF_NO_MT_BOX) == 0)
	{
		r.x = e->x - (size / 2) - battle.camera.x - 4;
		r.y = e->y - (size / 2) - battle.camera.y - 4;
		r.w = size + 8;
		r.h = size + 8;

		SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
		SDL_RenderDrawRect(app.renderer, &r);
	}
	
	if (e == battle.messageSpeaker && e != player && battle.stats[STAT_TIME] % 40 < 20)
	{
		r.x = e->x - (size / 2) - battle.camera.x;
		r.y = e->y - (size / 2) - battle.camera.y;
		r.w = size;
		r.h = size;

		SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(app.renderer, &r);
	}
}

void activateEntities(char *names)
{
	Entity *e;
	char *name;

	name = strtok(names, ";");

	while (name)
	{
		for (e = battle.entityHead.next ; e != NULL ; e = e->next)
		{
			if (strcmp(e->name, name) == 0)
			{
				e->active = 1;

				if (e->type == ET_CAPITAL_SHIP)
				{
					updateCapitalShipComponentProperties(e, 0);
				}
			}
		}

		name = strtok(NULL, ";");
	}

	notifyNewArrivals();
}

void activateEntityGroups(char *groupNames)
{
	Entity *e;
	char *groupName;

	groupName = strtok(groupNames, ";");

	while (groupName)
	{
		for (e = battle.entityHead.next ; e != NULL ; e = e->next)
		{
			if (strcmp(e->groupName, groupName) == 0)
			{
				SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Activated %s (%s)", e->name, groupName);
				
				e->active = 1;
				
				if (e->type == ET_CAPITAL_SHIP)
				{
					updateCapitalShipComponentProperties(e, 0);
				}
			}
		}

		groupName = strtok(NULL, ";");
	}

	notifyNewArrivals();
}

/*
 * Some craft, such as capital ships, might be performing a long action and won't notice new craft arrive for well over 30 seconds.
 * We'll knock the times down to a max of 1 second, so they can react faster.
 */
static void notifyNewArrivals(void)
{
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && (e->type == ET_FIGHTER || e->type == ET_CAPITAL_SHIP))
		{
			e->aiActionTime = MIN(e->aiActionTime, FPS);
		}
	}
}

static void activateEpicFighters(int side)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (!e->active && e->type == ET_FIGHTER && !(e->flags & EF_NO_EPIC) && ((side == SIDE_ALLIES && e->side == SIDE_ALLIES) || (side != SIDE_ALLIES && e->side != SIDE_ALLIES)))
		{
			e->active = 1;
			
			/* don't spring into existence in front of the player */
			if (isOnBattleScreen(e->x, e->y, e->w, e->h))
			{
				e->x = player->x;
				e->y = player->y;
				
				e->x += (rand() % 2) ? -SCREEN_WIDTH : SCREEN_WIDTH;
				e->y += (rand() % 2) ? -SCREEN_HEIGHT : SCREEN_HEIGHT;
			}
			
			return;
		}
	}
}

void countNumEnemies(void)
{
	Entity *e;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->side != SIDE_ALLIES && (e->type == ET_FIGHTER || e->type == ET_CAPITAL_SHIP) && (!(e->flags & EF_NO_THREAT)))
		{
			battle.numInitialEnemies++;
		}
	}

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "battle.numInitialEnemies=%d", battle.numInitialEnemies);
}

void addAllToQuadtree(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active)
		{
			addToQuadtree(e, &battle.quadtree);
		}
	}
}

static int drawComparator(const void *a, const void *b)
{
	Entity *e1 = *((Entity**)a);
	Entity *e2 = *((Entity**)b);

	return e2->type - e1->type;
}

void killEntity(char *name)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, name) == 0)
		{
			e->health = 0;
			e->deathType = DT_INSTANT;
			
			/* prevent objectives and conditions from firing */
			strcpy(e->name, "");
			strcpy(e->groupName, "");
		}
	}
}

void updateEntitySide(char *sideStr, char *name)
{
	Entity *e;
	int side;
	
	side = lookup(sideStr);
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, name) == 0)
		{
			e->side = side;
			
			if (e->side != player->side)
			{
				e->flags |= EF_MISSION_TARGET;
			}
		}
	}
}

void awardPandoranCraftTrophy(void)
{
	Entity *e;
	
	for (e = deadHead.next ; e != NULL ; e = e->next)
	{
		if (e->killedBy == player && e->side == SIDE_PANDORAN)
		{
			awardTrophy("PANDORAN");
		}
	}
}

void destroyEntities(void)
{
	Entity *e;

	while (deadHead.next)
	{
		e = deadHead.next;
		deadHead.next = e->next;
		free(e);
	}

	deadTail = &deadHead;
}
