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

#include "player.h"

static void selectTarget(void);
static void switchGuns(void);
static void selectMissionTarget(void);
static void selectNewPlayer(int dir);
static void initPlayerSelect(void);
static void activateBoost(void);
static void deactivateBoost(void);
static void activateECM(void);

static int selectedPlayerIndex;
static int availableGuns[BT_MAX];
static Entity *availablePlayerUnits[MAX_SELECTABLE_PLAYERS];

void initPlayer(void)
{
	int i, n;
	
	memset(&availableGuns, 0, sizeof(int) * BT_MAX);
	
	player->selectedGunType = -1;
		
	for (i = 0 ; i < MAX_FIGHTER_GUNS ; i++)
	{
		n = player->guns[i].type;
		
		if (n)
		{
			availableGuns[n] = 1;
			
			if (player->selectedGunType == -1)
			{
				player->selectedGunType = n;
			}
		}
	}
	
	STRNCPY(player->name, "Player", MAX_NAME_LENGTH);
	
	player->action = NULL;
	
	battle.boostTimer = BOOST_RECHARGE_TIME;
	battle.ecmTimer = ECM_RECHARGE_TIME;
	
	game.stats[STAT_EPIC_KILL_STREAK] = MAX(game.stats[STAT_EPIC_KILL_STREAK], battle.stats[STAT_EPIC_KILL_STREAK]);
	
	battle.stats[STAT_EPIC_KILL_STREAK] = 0;
}

void doPlayer(void)
{
	battle.boostTimer = MIN(battle.boostTimer + 1, BOOST_RECHARGE_TIME);
	battle.ecmTimer = MIN(battle.ecmTimer + 1, ECM_RECHARGE_TIME);
	
	if (player != NULL)
	{
		self = player;
		
		if (player->alive == ALIVE_ALIVE)
		{
			if (app.keyboard[SDL_SCANCODE_LEFT])
			{
				player->angle -= 4;
			}
			
			if (app.keyboard[SDL_SCANCODE_RIGHT])
			{
				player->angle += 4;
			}
			
			if (app.keyboard[SDL_SCANCODE_UP] && battle.boostTimer > BOOST_FINISHED_TIME)
			{
				applyFighterThrust();
			}
			
			if (app.keyboard[SDL_SCANCODE_DOWN])
			{
				applyFighterBrakes();
			}
			
			if (battle.status == MS_IN_PROGRESS)
			{
				if (app.keyboard[SDL_SCANCODE_LCTRL] && !player->reload && player->guns[0].type)
				{
					fireGuns(player);
				}
				
				if (app.keyboard[SDL_SCANCODE_LSHIFT])
				{
					switchGuns();
					
					app.keyboard[SDL_SCANCODE_LSHIFT] = 0;
				}
				
				if (app.keyboard[SDL_SCANCODE_RETURN] && player->missiles && player->target)
				{
					if (getDistance(player->x, player->y, player->target->x, player->target->y) <= SCREEN_WIDTH)
					{
						fireMissile(player);
					}
					else
					{
						addHudMessage(colors.white, "Target not in range");
					}
					
					app.keyboard[SDL_SCANCODE_RETURN] = 0;
				}
				
				if (!player->target || player->target->systemPower <= 0 || app.keyboard[SDL_SCANCODE_T])
				{
					selectTarget();
					
					app.keyboard[SDLK_t] = 0;
				}
				
				if (app.keyboard[SDL_SCANCODE_SPACE] && battle.boostTimer == BOOST_RECHARGE_TIME)
				{
					playSound(SND_BOOST);
					
					activateBoost();
				}
				
				if (app.keyboard[SDL_SCANCODE_E] && battle.ecmTimer == ECM_RECHARGE_TIME)
				{
					activateECM();
				}
				
				if (!battle.missionTarget)
				{
					selectMissionTarget();
				}
			}
		}
		
		player->angle = ((int)player->angle) % 360;
		
		if (player->health <= 0 && battle.status == MS_IN_PROGRESS)
		{
			if (!battle.epic)
			{
				failMission();
			}
			else if (player->health == -FPS)
			{
				initPlayerSelect();
			}
		}
	}
	
	if (battle.boostTimer == (int)BOOST_FINISHED_TIME)
	{
		deactivateBoost();
	}
}

void initPlayerSelect(void)
{
	Entity *e;
	
	memset(&availablePlayerUnits, 0, sizeof(Entity*) * MAX_SELECTABLE_PLAYERS);
	
	selectedPlayerIndex = 0;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->type == ET_FIGHTER && e->health > 0 && e->side == SIDE_ALLIES && selectedPlayerIndex < MAX_SELECTABLE_PLAYERS)
		{
			availablePlayerUnits[selectedPlayerIndex++] = e;
		}
	}
	
	if (selectedPlayerIndex > 0)
	{
		battle.playerSelect = 1;
		selectedPlayerIndex = 0;		
		memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	}
	else
	{
		battle.epic = 0;
		
		failMission();
	}
}

void doPlayerSelect(void)
{
	if (app.keyboard[SDL_SCANCODE_LEFT])
	{
		selectNewPlayer(-1);
		
		app.keyboard[SDL_SCANCODE_LEFT] = 0;
	}
	
	if (app.keyboard[SDL_SCANCODE_RIGHT])
	{
		selectNewPlayer(1);
		
		app.keyboard[SDL_SCANCODE_RIGHT] = 0;
	}
	
	if (app.keyboard[SDL_SCANCODE_RETURN] && player->health > 0)
	{
		battle.playerSelect = 0;
		
		initPlayer();
		
		app.keyboard[SDL_SCANCODE_RETURN] = 0;
	}
}

static void selectNewPlayer(int dir)
{
	do
	{
		selectedPlayerIndex += dir;
		
		selectedPlayerIndex = mod(selectedPlayerIndex, MAX_SELECTABLE_PLAYERS);
		
		player = availablePlayerUnits[selectedPlayerIndex];
	}
	while (player == NULL);
	
	battle.camera.x = player->x - (SCREEN_WIDTH / 2);
	battle.camera.y = player->y - (SCREEN_HEIGHT / 2);
}

static void activateBoost(void)
{
	self->dx += sin(TO_RAIDANS(self->angle)) * 10;
	self->dy += -cos(TO_RAIDANS(self->angle)) * 10;
	self->thrust = sqrt((self->dx * self->dx) + (self->dy * self->dy));
	
	battle.boostTimer = 0;
	
	battle.stats[STAT_BOOST]++;
}

static void deactivateBoost(void)
{
	float v, thrust;
	
	thrust = -1;
	
	while (thrust != self->thrust)
	{
		thrust = self->thrust;
		
		v = (self->speed / sqrt(self->thrust));
		self->dx = v * self->dx;
		self->dy = v * self->dy;
		self->thrust = sqrt((self->dx * self->dx) + (self->dy * self->dy));
	}
}

static void activateECM(void)
{
	battle.ecmTimer = 0;
	
	battle.stats[STAT_ECM]++;
}

static void switchGuns(void)
{
	int i;
	
	i = player->selectedGunType;
	
	do
	{
		i = (i + 1) % BT_MAX;
	}
	while (!availableGuns[i]);
	
	player->selectedGunType = i;
}

static void selectTarget(void)
{
	unsigned int closest = MAX_TARGET_RANGE;
	unsigned int dist = MAX_TARGET_RANGE;
	Entity *e;
	
	player->target = NULL;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e != player && e->type == ET_FIGHTER && e->side != player->side && e->alive == ALIVE_ALIVE && (!(e->flags & EF_DISABLED)))
		{
			dist = getDistance(self->x, self->y, e->x, e->y);
			if (dist < closest)
			{
				player->target = e;
				closest = dist;
			}
		}
	}
}

static void selectMissionTarget(void)
{
	unsigned int closest = MAX_TARGET_RANGE;
	unsigned int dist = MAX_TARGET_RANGE;
	Entity *e;
	
	battle.missionTarget = NULL;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->flags & EF_MISSION_TARGET && e->alive == ALIVE_ALIVE)
		{
			if (battle.missionTarget == NULL)
			{
				battle.missionTarget = e;
			}
			else if (battle.missionTarget->type == ET_WAYPOINT && e->type != ET_WAYPOINT)
			{
				battle.missionTarget = e;
			}
			else if (battle.missionTarget->type != ET_WAYPOINT)
			{
				dist = getDistance(self->x, self->y, e->x, e->y);
				if (dist < closest)
				{
					battle.missionTarget = e;
					closest = dist;
				}
			}
			else if (battle.missionTarget->type == ET_WAYPOINT && e->type == ET_WAYPOINT && e->id < battle.missionTarget->id)
			{
				battle.missionTarget = e;
			}
		}
	}
}
