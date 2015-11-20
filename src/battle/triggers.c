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

#include "triggers.h"

static int conditionMet(Trigger *trigger);
static void fireTrigger(Trigger *trigger);

void checkTrigger(char *name, int type)
{
	Trigger *trigger;
	
	for (trigger = battle.triggerHead.next ; trigger != NULL ; trigger = trigger->next)
	{
		if (trigger->type == type && strcmp(trigger->targetName, name) == 0 && conditionMet(trigger))
		{
			fireTrigger(trigger);
		}
	}
}

static int conditionMet(Trigger *trigger)
{
	switch (trigger->type)
	{
		case TRIGGER_TIME:
			return trigger->targetValue == battle.stats[STAT_TIME] / FPS;
			
		case TRIGGER_KILLS:
			return trigger->targetValue == battle.stats[STAT_ENEMIES_KILLED];
			
		case TRIGGER_LOSSES:
			return trigger->targetValue == battle.stats[STAT_ALLIES_KILLED];
			
		case TRIGGER_WAYPOINT:
			return 1;
			
		case TRIGGER_ESCAPES:
			return trigger->targetValue == battle.stats[STAT_ENEMIES_ESCAPED];
	}
	
	return 0;
}

static void fireTrigger(Trigger *trigger)
{
	switch (trigger->action)
	{
		case TA_COMPLETE_MISSION:
			addHudMessage(colors.green, "Mission Complete!");
			completeMission();
			break;
		
		case TA_FAIL_MISSION:
			addHudMessage(colors.red, "Mission Failed!");
			failMission();
			break;
			
		case TA_ACTIVE_ENTITY:
			activateEntities(trigger->actionValue);
			break;
			
		case TA_ACTIVE_ENTITY_GROUP:
			activateEntityGroup(trigger->actionValue);
			break;
			
		case TA_ACTIVE_OBJECTIVE:
			activateObjective(atoi(trigger->actionValue));
			break;
			
		case TA_RETREAT_ALLIES:
			battle.epic = 0;
			addHudMessage(colors.red, "Mission Aborted! Retreat!");
			retreatAllies();
			break;
	}
}
