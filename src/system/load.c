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

#include "load.h"

static void loadStats(cJSON *stats);
static void loadStarSystems(cJSON *starSystemsJSON);
static void loadMissions(cJSON *missionsCJSON);
static void loadChallenges(Mission *mission, cJSON *challengesCJSON);

void loadGame(void)
{
	cJSON *root, *gameJSON;
	char *text;
	
	text = readFile(getSaveFilePath("game.save"));
	root = cJSON_Parse(text);
	
	gameJSON = cJSON_GetObjectItem(root, "game");
	
	STRNCPY(game.selectedStarSystem, cJSON_GetObjectItem(gameJSON, "selectedStarSystem")->valuestring, MAX_NAME_LENGTH);
	
	loadStarSystems(cJSON_GetObjectItem(gameJSON, "starSystems"));
	
	loadStats(cJSON_GetObjectItem(gameJSON, "stats"));
	
	cJSON_Delete(root);
	free(text);
}

static void loadStarSystems(cJSON *starSystemsJSON)
{
	StarSystem *starSystem;
	cJSON *starSystemJSON;
	
	for (starSystemJSON = starSystemsJSON->child ; starSystemJSON != NULL ; starSystemJSON = starSystemJSON->next)
	{
		starSystem = getStarSystem(cJSON_GetObjectItem(starSystemJSON, "name")->valuestring);
		
		starSystem->side = lookup(cJSON_GetObjectItem(starSystemJSON, "side")->valuestring);
		
		loadMissions(cJSON_GetObjectItem(starSystemJSON, "missions"));
	}
}

static void loadMissions(cJSON *missionsCJSON)
{
	Mission *mission;
	cJSON *missionCJSON;
	
	for (missionCJSON = missionsCJSON->child ; missionCJSON != NULL ; missionCJSON = missionCJSON->next)
	{
		mission = getMission(cJSON_GetObjectItem(missionCJSON, "filename")->valuestring);
		
		mission->completed = cJSON_GetObjectItem(missionCJSON, "completed")->valueint;
		
		if (cJSON_GetObjectItem(missionCJSON, "challenges"))
		{
			loadChallenges(mission, cJSON_GetObjectItem(missionCJSON, "challenges"));
		}
	}
}

static void loadChallenges(Mission *mission, cJSON *challengesCJSON)
{
	Challenge *challenge;
	cJSON *challengeCJSON;
	
	for (challengeCJSON = challengesCJSON->child ; challengeCJSON != NULL ; challengeCJSON = challengeCJSON->next)
	{
		challenge = getChallenge(mission, lookup(cJSON_GetObjectItem(challengeCJSON, "type")->valuestring));
		
		if (!challenge)
		{
			printf("Couldn't find challenge to update\n");
			continue;
		}
		
		challenge->passed = cJSON_GetObjectItem(challengeCJSON, "passed")->valueint;
	}
}

static void loadStats(cJSON *stats)
{
	int i;
	char *statName;
	
	for (i = 0 ; i < STAT_MAX ; i++)
	{
		statName = getLookupName("STAT_", i);
		
		if (statName && cJSON_GetObjectItem(stats, statName))
		{
			game.stats[i] = cJSON_GetObjectItem(stats, statName)->valueint;
		}
	}
}
