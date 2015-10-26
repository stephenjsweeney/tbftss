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

#include "lookup.h"

static Lookup head;
static Lookup *tail;

static void addLookup(char *name, long value);

void initLookups(void)
{
	memset(&head, 0, sizeof(Lookup));
	tail = &head;
	
	addLookup("FF_NO_KILL", FF_NO_KILL);
	addLookup("FF_DISABLE", FF_DISABLE);
	addLookup("FF_IMMORTAL", FF_IMMORTAL);
	addLookup("FF_MISSION_TARGET", FF_MISSION_TARGET);
	
	addLookup("TT_DESTROY", TT_DESTROY);
	addLookup("TT_DISABLE", TT_DISABLE);
	
	addLookup("WT_BUTTON", WT_BUTTON);
	addLookup("WT_SELECT", WT_SELECT);
	
	addLookup("SIDE_ALLIES", SIDE_ALLIES);
	addLookup("SIDE_PIRATE", SIDE_PIRATE);
	addLookup("SIDE_PANDORAN", SIDE_PANDORAN);
	addLookup("SIDE_CSN", SIDE_CSN);
	addLookup("SIDE_UNF", SIDE_UNF);
	addLookup("SIDE_INF", SIDE_INF);
	
	addLookup("SND_PARTICLE", SND_PARTICLE);
	addLookup("SND_PLASMA", SND_PLASMA);
	addLookup("SND_MAG", SND_MAG);
	addLookup("SND_MISSILE", SND_MISSILE);
	
	addLookup("BT_PARTICLE", BT_PARTICLE);
	addLookup("BT_PLASMA", BT_PLASMA);
	addLookup("BT_LASER", BT_LASER);
	addLookup("BT_MAG", BT_MAG);
	addLookup("BT_ROCKET", BT_ROCKET);
	addLookup("BT_MISSILE", BT_MISSILE);
	
	addLookup("BF_NONE", BF_NONE);
	addLookup("BF_ENGINE", BF_ENGINE);
	addLookup("BF_SYSTEM_DAMAGE", BF_SYSTEM_DAMAGE);
	addLookup("BF_EXPLODES", BF_EXPLODES);
	
	addLookup("MISSILE_ROCKET", MISSILE_ROCKET);
	addLookup("MISSILE_MISSILE", MISSILE_MISSILE);
	addLookup("MISSILE_FF", MISSILE_FF);
	
	addLookup("CHALLENGE_ARMOUR", CHALLENGE_ARMOUR);
	addLookup("CHALLENGE_TIME", CHALLENGE_TIME);
	addLookup("CHALLENGE_ACCURACY", CHALLENGE_ACCURACY);
	addLookup("CHALLENGE_NO_LOSSES", CHALLENGE_NO_LOSSES);
	addLookup("CHALLENGE_1_LOSS", CHALLENGE_1_LOSS);
	addLookup("CHALLENGE_LOSSES", CHALLENGE_LOSSES);
	addLookup("CHALLENGE_PLAYER_KILLS", CHALLENGE_PLAYER_KILLS);
	addLookup("CHALLENGE_DISABLE", CHALLENGE_DISABLE);
	addLookup("CHALLENGE_TIME_MINS", CHALLENGE_TIME_MINS);
	
	addLookup("STAT_MISSIONS_STARTED", STAT_MISSIONS_STARTED);
	addLookup("STAT_MISSIONS_COMPLETED", STAT_MISSIONS_COMPLETED);
	addLookup("STAT_SHOTS_FIRED", STAT_SHOTS_FIRED);
	addLookup("STAT_SHOTS_HIT", STAT_SHOTS_HIT);
	addLookup("STAT_MISSILES_FIRED", STAT_MISSILES_FIRED);
	addLookup("STAT_MISSILES_HIT", STAT_MISSILES_HIT);
	addLookup("STAT_ENEMIES_KILLED", STAT_ENEMIES_KILLED);
	addLookup("STAT_ENEMIES_KILLED_PLAYER", STAT_ENEMIES_KILLED_PLAYER);
	addLookup("STAT_ALLIES_KILLED", STAT_ALLIES_KILLED);
	addLookup("STAT_PLAYER_KILLED", STAT_PLAYER_KILLED);
	addLookup("STAT_DISABLED", STAT_DISABLED);
	addLookup("STAT_TIME", STAT_TIME);
	
	addLookup("TRIGGER_TIME", TRIGGER_TIME);
	addLookup("TRIGGER_KILLS", TRIGGER_KILLS);
	
	addLookup("TA_COMPLETE_MISSION", TA_COMPLETE_MISSION);
	addLookup("TA_FAIL_MISSION", TA_FAIL_MISSION);
}

static void addLookup(char *name, long value)
{
	Lookup *lookup = malloc(sizeof(Lookup));
	memset(lookup, 0, sizeof(Lookup));
	
	STRNCPY(lookup->name, name, MAX_NAME_LENGTH);
	lookup->value = value;
	
	tail->next = lookup;
	tail = lookup;
}

long lookup(char *name)
{
	Lookup *l;
	
	for (l = head.next ; l != NULL ; l = l->next)
	{
		if (strcmp(l->name, name) == 0)
		{
			return l->value;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "No such lookup value '%s'", name);
	
	exit(1);
	
	return 0;
}

char *getLookupName(char *prefix, long num)
{
	Lookup *l;
	
	for (l = head.next ; l != NULL ; l = l->next)
	{
		if (l->value == num && strncmp(prefix, l->name, strlen(prefix)) == 0)
		{
			return l->name;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "No such lookup value %d, prefix=%s", num, prefix);
	
	exit(1);
	
	return "";
}

char *getFlagValues(char *prefix, long flags)
{
	static char flagStr[MAX_DESCRIPTION_LENGTH];
	int requirePlus;
	Lookup *l;
	
	memset(flagStr, '\0', MAX_DESCRIPTION_LENGTH);
	
	requirePlus = 0;
	
	for (l = head.next ; l != NULL ; l = l->next)
	{
		if (flags & l->value && strncmp(prefix, l->name, strlen(prefix)) == 0)
		{
			if (requirePlus)
			{
				strcat(flagStr, "+");
			}
			
			strcat(flagStr, l->name);
			
			requirePlus = 1;
		}
	}
	
	return flagStr;
}

long flagsToLong(char *flags)
{
	char *flag;
	long total;
	
	total = 0;
	
	flag = strtok(flags, "+");
	while (flag)
	{
		total += lookup(flag);
		flag = strtok(NULL, "+");
	}
	
	return total;
}

void destroyLookups(void)
{
	Lookup *l;
	
	while (head.next)
	{
		l = head.next;
		head.next = l->next;
		free(l);
	}
}
