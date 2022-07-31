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

#include "../battle/entities.h"
#include "../battle/hud.h"
#include "../battle/objectives.h"
#include "../battle/script.h"
#include "../system/atlas.h"
#include "../system/sound.h"
#include "../system/util.h"
#include "waypoints.h"

extern Battle  battle;
extern Colors  colors;
extern Entity *player;
extern Entity *self;
extern Game	   game;

static void think(void);
static int	teamMatesClose(void);
static int	isCurrentObjective(void);

static int waypointId;
static int currentWaypointId;

void resetWaypoints(void)
{
	waypointId = 1;
	currentWaypointId = 0;
}

Entity *spawnWaypoint(void)
{
	Entity *waypoint = spawnEntity();

	sprintf(waypoint->name, "Waypoint #%d", waypointId);
	waypoint->id = waypointId;
	waypoint->type = ET_WAYPOINT;
	waypoint->active = 0;
	waypoint->health = waypoint->maxHealth = FPS;
	waypoint->texture = getAtlasImage("gfx/entities/waypoint.png");
	waypoint->flags = EF_NO_MT_BOX + EF_MISSION_TARGET + EF_NO_HEALTH_BAR;
	waypoint->action = think;

	waypoint->w = waypoint->texture->rect.w;
	waypoint->h = waypoint->texture->rect.h;

	waypointId++;

	return waypoint;
}

static void think(void)
{
	self->angle += 0.25;

	if (self->angle >= 360)
	{
		self->angle -= 360;
	}

	if (--self->aiActionTime <= 0)
	{
		self->aiActionTime = 0;

		if (self->health && player->alive == ALIVE_ALIVE && getDistance(player->x, player->y, self->x, self->y) <= 128 && isCurrentObjective() && teamMatesClose())
		{
			self->health = 0;

			updateObjective("Waypoint", TT_WAYPOINT);

			runScriptFunction(self->name);

			if (battle.waypointAutoAdvance)
			{
				activateNextWaypoint();
			}

			battle.stats[STAT_WAYPOINTS_VISITED]++;

			playSound(SND_WAYPOINT);
		}
	}
}

static int isCurrentObjective(void)
{
	int numActiveObjectives = battle.numObjectivesTotal - battle.numObjectivesComplete;

	if (numActiveObjectives > 1)
	{
		addHudMessage(colors.cyan, _("Cannot activate waypoint - outstanding objectives not yet complete"));
		self->aiActionTime = FPS;
		return 0;
	}

	if (game.currentMission->challengeData.isChallenge && game.currentMission->challengeData.clearWaypointEnemies && battle.numEnemies > 0)
	{
		addHudMessage(colors.cyan, _("Cannot activate waypoint - eliminate enemies first"));
		self->aiActionTime = FPS;
		return 0;
	}

	return 1;
}

static int teamMatesClose(void)
{
	Entity *e;

	if (player->side != SIDE_PANDORAN)
	{
		for (e = battle.entityHead.next; e != NULL; e = e->next)
		{
			if (e->active && e->type == ET_FIGHTER && e->side == SIDE_ALLIES)
			{
				if (getDistance(player->x, player->y, e->x, e->y) > 350)
				{
					addHudMessage(colors.cyan, _("Cannot activate waypoint - team mates too far away"));
					self->aiActionTime = FPS;
					return 0;
				}
			}
		}
	}

	return 1;
}

void activateNextWaypoint(void)
{
	Entity *e;
	Entity *nextWaypoint = NULL;

	currentWaypointId++;

	for (e = battle.entityHead.next; e != NULL; e = e->next)
	{
		if (e->type == ET_WAYPOINT && e->id == currentWaypointId)
		{
			nextWaypoint = e;
		}
	}

	if (nextWaypoint != NULL)
	{
		nextWaypoint->active = 1;

		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Activating %s", nextWaypoint->name);
	}
}
