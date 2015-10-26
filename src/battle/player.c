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

static int availableGuns[BT_MAX];

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
}

void doPlayer(void)
{
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
			
			if (app.keyboard[SDL_SCANCODE_UP])
			{
				applyFighterThrust();
			}
			
			if (app.keyboard[SDL_SCANCODE_DOWN])
			{
				applyFighterBrakes();
			}
			
			if (app.keyboard[SDL_SCANCODE_LCTRL] && !player->reload && player->guns[0].type)
			{
				fireGuns(player);
			}
			
			if (app.keyboard[SDL_SCANCODE_LSHIFT])
			{
				switchGuns();
				
				app.keyboard[SDL_SCANCODE_LSHIFT] = 0;
			}
			
			if (app.keyboard[SDL_SCANCODE_RETURN] && player->missiles.ammo && player->target)
			{
				fireMissile(player);
				
				app.keyboard[SDL_SCANCODE_RETURN] = 0;
			}
			
			if (!player->target || player->target->health <= 0 || player->target->systemPower <= 0 || getDistance(player->x, player->y, player->target->x, player->target->y) > 1000)
			{
				selectTarget();
			}
		}
		
		player->angle = ((int)player->angle) % 360;
		
		player->x = SCREEN_WIDTH / 2;
		player->y = SCREEN_HEIGHT / 2;
		
		if (player->health <= 0 && battle.status == MS_IN_PROGRESS)
		{
			failIncompleteObjectives();
			
			battle.status = MS_FAILED;
			battle.missionFinishedTimer = FPS;
		}
	}
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
	unsigned int closest = 65535;
	unsigned int dist = 65535;
	Entity *f;
	
	player->target = NULL;
	
	for (f = battle.entityHead.next ; f != NULL ; f = f->next)
	{
		if (f != player && f->side != SIDE_NONE && f->side != player->side && f->alive == ALIVE_ALIVE)
		{
			dist = getDistance(self->x, self->y, f->x, f->y);
			if (dist < closest)
			{
				player->target = f;
				closest = dist;
			}
		}
	}
}
