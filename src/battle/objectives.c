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

#include "objectives.h"

void failIncompleteObjectives(void);

void doObjectives(void)
{
	Objective *o;
	
	battle.numObjectivesComplete = battle.numObjectivesTotal = 0;
	
	for (o = battle.objectiveHead.next ; o != NULL ; o = o->next)
	{
		battle.numObjectivesTotal++;
		
		if (o->currentValue == o->targetValue)
		{
			battle.numObjectivesComplete++;
		}
	}
	
	if (battle.numObjectivesTotal > 0 && battle.numObjectivesComplete == battle.numObjectivesTotal && battle.status == MS_IN_PROGRESS)
	{
		battle.status = MS_COMPLETE;
		battle.missionFinishedTimer = FPS;
		
		game.stats.missionsCompleted++;
		
		updateChallenges();
	}
}

void updateObjective(char *name)
{
	Objective *o;
	
	for (o = battle.objectiveHead.next ; o != NULL ; o = o->next)
	{
		if (strcmp(o->targetName, name) == 0)
		{
			o->currentValue = MIN(o->targetValue, o->currentValue + 1);
			
			if (o->targetValue - o->currentValue <= 10)
			{
				addHudMessage(colors.cyan, "%s - %d / %d", o->description, o->currentValue, o->targetValue);
			}
			else if (o->currentValue % 10 == 0)
			{
				addHudMessage(colors.cyan, "%s - %d / %d", o->description, o->currentValue, o->targetValue);
			}
			
			if (o->currentValue == o->targetValue)
			{
				switch (o->status)
				{
					case OS_INCOMPLETE:
						o->status = OS_COMPLETE;
						addHudMessage(colors.green, "%s - Objective Complete!", o->description);
						break;
						
					case OS_CONDITIONAL:
						o->status = OS_FAILED;
						battle.status = MS_FAILED;
						battle.missionFinishedTimer = FPS;
						failIncompleteObjectives();
						break;
				}
			}
		}
	}
}

void failIncompleteObjectives(void)
{
	Objective *o;
	
	for (o = battle.objectiveHead.next ; o != NULL ; o = o->next)
	{
		if (o->status != OS_COMPLETE)
		{
			o->status = OS_FAILED;
		}
	}
}
