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

#include "game.h"

void initGame(void)
{
	memset(&game, 0, sizeof(Game));
	
	STRNCPY(game.selectedStarSystem, "Sol", MAX_NAME_LENGTH);
}

void resetCampaign(int difficulty)
{
	StarSystem *starSystem;
	Mission *mission;
	
	game.difficulty = difficulty;
	
	STRNCPY(game.selectedStarSystem, "Sol", MAX_NAME_LENGTH);
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		starSystem->completedMissions = 0;
		starSystem->activeMission = NULL;
		
		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			mission->available = 0;
			mission->completed = 0;
		}
	}
}

void destroyGame(void)
{
}
