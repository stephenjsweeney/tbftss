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

#include "lookup.h"

static Lookup head;
static Lookup *tail;

static void addLookup(char *name, long value); 

void initLookups(void)
{
	memset(&head, 0, sizeof(Lookup));
	tail = &head;

	addLookup("CONTROL_FIRE", CONTROL_FIRE);
	addLookup("CONTROL_ACCELERATE", CONTROL_ACCELERATE);
	addLookup("CONTROL_BOOST", CONTROL_BOOST);
	addLookup("CONTROL_ECM", CONTROL_ECM);
	addLookup("CONTROL_BRAKE", CONTROL_BRAKE);
	addLookup("CONTROL_TARGET", CONTROL_TARGET);
	addLookup("CONTROL_MISSILE", CONTROL_MISSILE);
	addLookup("CONTROL_GUNS", CONTROL_GUNS);
	addLookup("CONTROL_RADAR", CONTROL_RADAR);
	addLookup("CONTROL_NEXT_FIGHTER", CONTROL_NEXT_FIGHTER);
	addLookup("CONTROL_PREV_FIGHTER", CONTROL_PREV_FIGHTER);
	addLookup("CONTROL_SCREENSHOT", CONTROL_SCREENSHOT);
	
	addLookup("ET_WAYPOINT", ET_WAYPOINT);
	addLookup("ET_JUMPGATE", ET_JUMPGATE);
	addLookup("ET_CAPITAL_SHIP", ET_CAPITAL_SHIP);
	addLookup("ET_MINE", ET_MINE);
	addLookup("ET_SHADOW_MINE", ET_SHADOW_MINE);
	
	addLookup("EF_NO_KILL", EF_NO_KILL);
	addLookup("EF_DISABLED", EF_DISABLED);
	addLookup("EF_MUST_DISABLE", EF_MUST_DISABLE);
	addLookup("EF_IMMORTAL", EF_IMMORTAL);
	addLookup("EF_MISSION_TARGET", EF_MISSION_TARGET);
	addLookup("EF_RETREATING", EF_RETREATING);
	addLookup("EF_HAS_ROPE", EF_HAS_ROPE);
	addLookup("EF_COLLECTS_ITEMS", EF_COLLECTS_ITEMS);
	addLookup("EF_NO_EPIC", EF_NO_EPIC);
	addLookup("EF_STATIC", EF_STATIC);
	addLookup("EF_TAKES_DAMAGE", EF_TAKES_DAMAGE);
	addLookup("EF_SECONDARY_TARGET", EF_SECONDARY_TARGET);
	addLookup("EF_AI_TARGET", EF_AI_TARGET);
	addLookup("EF_AI_LEADER", EF_AI_LEADER);
	addLookup("EF_NO_KILL_INC", EF_NO_KILL_INC);
	addLookup("EF_NO_MT_BOX", EF_NO_MT_BOX);
	addLookup("EF_FRIENDLY_HEALTH_BAR", EF_FRIENDLY_HEALTH_BAR);
	addLookup("EF_AI_IGNORE", EF_AI_IGNORE);
	addLookup("EF_NO_THREAT", EF_NO_THREAT);
	addLookup("EF_NO_HEALTH_BAR", EF_NO_HEALTH_BAR);
	addLookup("EF_DROPS_ITEMS", EF_DROPS_ITEMS);

	addLookup("AIF_NONE", AIF_NONE);
	addLookup("AIF_MOVES_TO_PLAYER", AIF_MOVES_TO_PLAYER);
	addLookup("AIF_FOLLOWS_PLAYER", AIF_FOLLOWS_PLAYER);
	addLookup("AIF_UNLIMITED_RANGE", AIF_UNLIMITED_RANGE);
	addLookup("AIF_COLLECTS_ITEMS", AIF_COLLECTS_ITEMS);
	addLookup("AIF_TOWS", AIF_TOWS);
	addLookup("AIF_RETREATS", AIF_RETREATS);
	addLookup("AIF_GOAL_JUMPGATE", AIF_GOAL_JUMPGATE);
	addLookup("AIF_AVOIDS_COMBAT", AIF_AVOIDS_COMBAT);
	addLookup("AIF_DEFENSIVE", AIF_DEFENSIVE);
	addLookup("AIF_MISSILE_BOAT", AIF_MISSILE_BOAT);
	addLookup("AIF_AGGRESSIVE", AIF_AGGRESSIVE);
	addLookup("AIF_LONG_RANGE_FIRE", AIF_LONG_RANGE_FIRE);
	addLookup("AIF_MOVES_TO_LEADER", AIF_MOVES_TO_LEADER);
	addLookup("AIF_WANDERS", AIF_WANDERS);
	addLookup("AIF_COVERS_RETREAT", AIF_COVERS_RETREAT);
	addLookup("AIF_TARGET_FOCUS", AIF_TARGET_FOCUS);
	addLookup("AIF_DROPS_MINES", AIF_DROPS_MINES);
	addLookup("AIF_ASSASSIN", AIF_ASSASSIN);
	addLookup("AIF_SUSPICIOUS", AIF_SUSPICIOUS);

	addLookup("DT_ANY", DT_ANY);
	addLookup("DT_NO_SPIN", DT_NO_SPIN);
	addLookup("DT_INSTANT", DT_INSTANT);
	addLookup("DT_SIMPLE", DT_SIMPLE);

	addLookup("TT_DESTROY", TT_DESTROY);
	addLookup("TT_DISABLE", TT_DISABLE);
	addLookup("TT_WAYPOINT", TT_WAYPOINT);
	addLookup("TT_ESCAPED", TT_ESCAPED);
	addLookup("TT_PLAYER_ESCAPED", TT_PLAYER_ESCAPED);
	addLookup("TT_ITEM", TT_ITEM);
	addLookup("TT_STOLEN", TT_STOLEN);
	addLookup("TT_MISC", TT_MISC);

	addLookup("WT_BUTTON", WT_BUTTON);
	addLookup("WT_SELECT", WT_SELECT);
	addLookup("WT_IMG_BUTTON", WT_IMG_BUTTON);
	addLookup("WT_CONTROL_CONFIG", WT_CONTROL_CONFIG);

	addLookup("SIDE_NONE", SIDE_NONE);
	addLookup("SIDE_ALLIES", SIDE_ALLIES);
	addLookup("SIDE_PIRATE", SIDE_PIRATE);
	addLookup("SIDE_REBEL", SIDE_REBEL);
	addLookup("SIDE_MERC", SIDE_MERC);
	addLookup("SIDE_PANDORAN", SIDE_PANDORAN);
	addLookup("SIDE_CSN", SIDE_CSN);
	addLookup("SIDE_UNF", SIDE_UNF);

	addLookup("SND_PARTICLE", SND_PARTICLE);
	addLookup("SND_PLASMA", SND_PLASMA);
	addLookup("SND_LASER", SND_LASER);
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
	addLookup("BF_SHIELD_DAMAGE", BF_SHIELD_DAMAGE);
	addLookup("BF_EXPLODES", BF_EXPLODES);

	addLookup("CHALLENGE_ARMOUR", CHALLENGE_ARMOUR);
	addLookup("CHALLENGE_TIME", CHALLENGE_TIME);
	addLookup("CHALLENGE_SURVIVE", CHALLENGE_SURVIVE);
	addLookup("CHALLENGE_SHOT_ACCURACY", CHALLENGE_SHOT_ACCURACY);
	addLookup("CHALLENGE_ROCKET_ACCURACY", CHALLENGE_ROCKET_ACCURACY);
	addLookup("CHALLENGE_MISSILE_ACCURACY", CHALLENGE_MISSILE_ACCURACY);
	addLookup("CHALLENGE_NO_LOSSES", CHALLENGE_NO_LOSSES);
	addLookup("CHALLENGE_1_LOSS", CHALLENGE_1_LOSS);
	addLookup("CHALLENGE_LOSSES", CHALLENGE_LOSSES);
	addLookup("CHALLENGE_PLAYER_KILLS", CHALLENGE_PLAYER_KILLS);
	addLookup("CHALLENGE_DISABLE", CHALLENGE_DISABLE);
	addLookup("CHALLENGE_ITEMS", CHALLENGE_ITEMS);
	addLookup("CHALLENGE_PLAYER_ITEMS", CHALLENGE_PLAYER_ITEMS);
	addLookup("CHALLENGE_RESCUE", CHALLENGE_RESCUE);

	addLookup("STAT_PERCENT_COMPLETE", STAT_PERCENT_COMPLETE);
	addLookup("STAT_MISSIONS_STARTED", STAT_MISSIONS_STARTED);
	addLookup("STAT_MISSIONS_COMPLETED", STAT_MISSIONS_COMPLETED);
	addLookup("STAT_OPTIONAL_COMPLETED", STAT_OPTIONAL_COMPLETED);
	addLookup("STAT_CHALLENGES_STARTED", STAT_CHALLENGES_STARTED);
	addLookup("STAT_CHALLENGES_COMPLETED", STAT_CHALLENGES_COMPLETED);
	addLookup("STAT_SHOTS_FIRED", STAT_SHOTS_FIRED);
	addLookup("STAT_SHOTS_HIT", STAT_SHOTS_HIT);
	addLookup("STAT_SHOT_ACCURACY", STAT_SHOT_ACCURACY);
	addLookup("STAT_ROCKETS_FIRED", STAT_ROCKETS_FIRED);
	addLookup("STAT_ROCKETS_HIT", STAT_ROCKETS_HIT);
	addLookup("STAT_ROCKET_ACCURACY", STAT_ROCKET_ACCURACY);
	addLookup("STAT_MISSILES_FIRED", STAT_MISSILES_FIRED);
	addLookup("STAT_MISSILES_HIT", STAT_MISSILES_HIT);
	addLookup("STAT_MISSILE_ACCURACY", STAT_MISSILE_ACCURACY);
	addLookup("STAT_ENEMIES_KILLED", STAT_ENEMIES_KILLED);
	addLookup("STAT_ENEMIES_KILLED_PLAYER", STAT_ENEMIES_KILLED_PLAYER);
	addLookup("STAT_ALLIES_KILLED", STAT_ALLIES_KILLED);
	addLookup("STAT_PLAYER_KILLED", STAT_PLAYER_KILLED);
	addLookup("STAT_ENEMIES_DISABLED", STAT_ENEMIES_DISABLED);
	addLookup("STAT_ENEMIES_ESCAPED", STAT_ENEMIES_ESCAPED);
	addLookup("STAT_CIVILIANS_RESCUED", STAT_CIVILIANS_RESCUED);
	addLookup("STAT_ECM", STAT_ECM);
	addLookup("STAT_BOOST", STAT_BOOST);
	addLookup("STAT_MISSILES_EVADED", STAT_MISSILES_EVADED);
	addLookup("STAT_MISSILES_STRUCK", STAT_MISSILES_STRUCK);
	addLookup("STAT_CIVILIANS_KILLED", STAT_CIVILIANS_KILLED);
	addLookup("STAT_TUG", STAT_TUG);
	addLookup("STAT_SHUTTLE", STAT_SHUTTLE);
	addLookup("STAT_NUM_TOWED", STAT_NUM_TOWED);
	addLookup("STAT_ITEMS_COLLECTED", STAT_ITEMS_COLLECTED);
	addLookup("STAT_ITEMS_COLLECTED_PLAYER", STAT_ITEMS_COLLECTED_PLAYER);
	addLookup("STAT_WAYPOINTS_VISITED", STAT_WAYPOINTS_VISITED);
	addLookup("STAT_EPIC_KILL_STREAK", STAT_EPIC_KILL_STREAK);
	addLookup("STAT_CAPITAL_SHIPS_DESTROYED", STAT_CAPITAL_SHIPS_DESTROYED);
	addLookup("STAT_CAPITAL_SHIPS_LOST", STAT_CAPITAL_SHIPS_LOST);
	addLookup("STAT_MINES_DESTROYED", STAT_MINES_DESTROYED);
	addLookup("STAT_TIME", STAT_TIME);

	addLookup("TROPHY_BRONZE", TROPHY_BRONZE);
	addLookup("TROPHY_SILVER", TROPHY_SILVER);
	addLookup("TROPHY_GOLD", TROPHY_GOLD);
	addLookup("TROPHY_PLATINUM", TROPHY_PLATINUM);
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

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "No such lookup value %ld, prefix=%s", num, prefix);

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

long flagsToLong(char *in, int *add)
{
	char *flag, *flags;
	long total;
	
	total = 0;

	if (add)
	{
		*add = (in[0] == '+');
	}
	
	flags = malloc(strlen(in) + 1);
	STRNCPY(flags, in, strlen(in) + 1);

	flag = strtok(flags, "+");
	while (flag)
	{
		total += lookup(flag);
		flag = strtok(NULL, "+");
	}
	
	free(flags);

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
