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

#include "waypoints.h"

static void think(void);
static int teamMatesClose(void);
static int isCurrentObjective(void);
void activateNextWaypoint(int id);

static int waypointId;

void resetWaypoints(void)
{
	waypointId = 1;
}

Entity *spawnWaypoint(void)
{
	Entity *waypoint = spawnEntity();
	
	sprintf(waypoint->name, "Waypoint #%d", waypointId++);
	waypoint->type = ET_WAYPOINT;
	waypoint->active = 0;
	waypoint->health = waypoint->maxHealth = FPS;
	waypoint->texture = getTexture("gfx/entities/waypoint.png");
	waypoint->flags = EF_MISSION_TARGET;
	waypoint->action = think;
	waypoint->flags |= EF_NO_MT_BOX;
	
	return waypoint;
}

static void think(void)
{
	self->thinkTime = 4;
	
	self->angle++;
	if (self->angle >= 360)
	{
		self->angle -= 360;
	}
	
	if (player != NULL && getDistance(player->x, player->y, self->x, self->y) <= 64 && isCurrentObjective() && teamMatesClose())
	{
		self->health = 0;
		
		updateObjective("Waypoint", TT_WAYPOINT);
		
		runScriptFunction(self->name);
		
		activateNextWaypoint(self->id);
	}
}

static int isCurrentObjective(void)
{
	int numActiveObjectives = battle.numObjectivesTotal - battle.numObjectivesComplete;
	
	if (numActiveObjectives > 1)
	{
		addHudMessage(colors.cyan, _("Cannot activate waypoint - outstanding objectives not yet complete"));
		self->thinkTime = FPS;
		return 0;
	}
	
	return 1;
}

static int teamMatesClose(void)
{
	Entity *e;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->type == ET_FIGHTER && e->side == SIDE_ALLIES)
		{
			if (getDistance(player->x, player->y, e->x, e->y) > 350)
			{
				addHudMessage(colors.cyan, _("Cannot activate waypoint - team mates too far away"));
				self->thinkTime = FPS;
				return 0;
			}
		}
	}
	
	return 1;
}

void activateNextWaypoint(int id)
{
	Entity *e;
	Entity *nextWaypoint = NULL;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->type == ET_WAYPOINT && e->id > id && (nextWaypoint == NULL || e->id < nextWaypoint->id))
		{
			nextWaypoint = e;
		}
	}
	
	if (nextWaypoint != NULL)
	{
		nextWaypoint->active = 1;
	}
}
