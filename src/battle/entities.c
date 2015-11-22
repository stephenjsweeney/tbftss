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

#include "entities.h"

static void drawEntity(Entity *e);
static void doEntity(void);
static void drawEntity(Entity *e);
static void activateEpicFighters(int n, int side);
static void restrictToGrid(Entity *e);
static void drawTargetRects(Entity *e);
static int drawComparator(const void *a, const void *b);

Entity *spawnEntity(void)
{
	Entity *e = malloc(sizeof(Entity));
	memset(e, 0, sizeof(Entity));
	e->id = battle.entId++;
	e->active = 1;
	
	battle.entityTail->next = e;
	battle.entityTail = e;
	
	return e;
}

void doEntities(void)
{
	int numAllies, numEnemies;
	int numActiveAllies, numActiveEnemies;
	Entity *e, *prev;
	
	prev = &battle.entityHead;
	
	numAllies = numEnemies = numActiveAllies = numActiveEnemies = 0;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active)
		{
			self = e;
			
			removeFromGrid(e);
			
			switch (e->type)
			{
				case ET_FIGHTER:
					doFighter();
					
					if (e->health > 0)
					{
						if (e->side == SIDE_ALLIES)
						{
							numActiveAllies++;
						}
						else
						{
							numActiveEnemies++;
						}
					}
					
					break;
					
				default:
					doEntity();
					break;
			}
			
			if (e->alive == ALIVE_ALIVE || e->alive == ALIVE_DYING)
			{
				if (e->action != NULL)
				{
					if (--e->thinkTime <= 0)
					{
						e->thinkTime = 0;
						e->action();
					}
				}
				
				doRope(e);
				
				restrictToGrid(e);
				
				e->x += e->dx;
				e->y += e->dy;
				
				addToGrid(e);
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
				
				if (e == player)
				{
					player = NULL;
					
					battle.playerSelect = battle.epic;
				}
				
				cutRope(e);
				
				prev->next = e->next;
				free(e);
				e = prev;
			}
		}
		
		if (e->type == ET_FIGHTER && (battle.epic || e->active) && !(e->flags & EF_NO_EPIC))
		{
			if (e->side == SIDE_ALLIES)
			{
				numAllies++;
			}
			else
			{
				numEnemies++;
			}
		}
		
		prev = e;
	}
	
	battle.numAllies = numAllies;
	battle.numEnemies = numEnemies;
	
	if (!battle.numInitialEnemies)
	{
		battle.numInitialEnemies = battle.numEnemies;
	}
	
	if (battle.epic && battle.stats[STAT_TIME] % FPS == 0)
	{
		if (numAllies > battle.epicFighterLimit)
		{
			activateEpicFighters(battle.epicFighterLimit - numActiveAllies, SIDE_ALLIES);
		}
		
		if (numEnemies > battle.epicFighterLimit)
		{
			activateEpicFighters(battle.epicFighterLimit - numActiveEnemies, SIDE_NONE);
		}
	}
}

static void restrictToGrid(Entity *e)
{
	float force;
	
	if (e->x <= GRID_RESTRICTION_SIZE)
	{
		force = GRID_RESTRICTION_SIZE - e->x;
		e->dx += force * 0.001;
		e->dx *= 0.95;
	}
	
	if (e->y <= GRID_RESTRICTION_SIZE)
	{
		force = GRID_RESTRICTION_SIZE - e->y;
		e->dy += force * 0.001;
		e->dy *= 0.95;
	}
	
	if (e->x >= (GRID_SIZE * GRID_CELL_WIDTH) - GRID_RESTRICTION_SIZE)
	{
		force = e->x - ((GRID_SIZE * GRID_CELL_WIDTH) - GRID_RESTRICTION_SIZE);
		e->dx -= force * 0.001;
		e->dx *= 0.95;
	}
	
	if (e->y >= (GRID_SIZE * GRID_CELL_HEIGHT) - GRID_RESTRICTION_SIZE)
	{
		force = e->y - ((GRID_SIZE * GRID_CELL_HEIGHT) - GRID_RESTRICTION_SIZE);
		e->dy -= force * 0.001;
		e->dy *= 0.95;
	}
}

static void doEntity(void)
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

void drawEntities(void)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsWithin(battle.camera.x, battle.camera.y, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);
	
	/* count number of candidates for use with qsort */
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i]) {}
	
	qsort(candidates, i, sizeof(Entity*), drawComparator);
	
	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->active)
		{
			switch (e->type)
			{
				case ET_FIGHTER:
					drawFighter(e);
					break;
					
				default:
					drawEntity(e);
					break;
			}
		}
		
		drawTargetRects(e);
		
		drawRope(e);
	}
}

static void drawEntity(Entity *e)
{
	blitRotated(e->texture, e->x - battle.camera.x, e->y - battle.camera.y, e->angle);
}

static void drawTargetRects(Entity *e)
{
	SDL_Rect r;
	
	int size = MAX(e->w, e->h) + 16;
	
	if (player != NULL && e == player->target)
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
}

void activateEntities(char *name)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, name) == 0)
		{
			e->active = 1;
		}
	}
}

void activateEntityGroup(char *groupName)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->groupName, groupName) == 0)
		{
			e->active = 1;
		}
	}
}

static void activateEpicFighters(int n, int side)
{
	Entity *e;
	
	if (n > 0)
	{
		for (e = battle.entityHead.next ; e != NULL ; e = e->next)
		{
			if (!e->active && e->type == ET_FIGHTER && !(e->flags & EF_NO_EPIC) && ((side == SIDE_ALLIES && e->side == SIDE_ALLIES) || (side != SIDE_ALLIES && e->side != SIDE_ALLIES)))
			{
				e->active = 1;
				
				if (--n <= 0)
				{
					return;
				}
			}
		}
	}
}

static int drawComparator(const void *a, const void *b)
{
	Entity *e1 = *((Entity**)a);
	Entity *e2 = *((Entity**)b);
	
	return e2->type - e1->type;
}
