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

#include "load.h"

static void loadStats(cJSON *stats);
static void loadStarSystems(cJSON *starSystemsJSON);
static void loadMissions(StarSystem *starSystem, cJSON *missionsCJSON);
static void loadChallenges(Mission *mission, cJSON *challengesCJSON);

void loadGame(void)
{
	cJSON *root, *game;
	char *text;
	
	text = readFile(getSaveFilePath("game.save"));
	root = cJSON_Parse(text);
	
	game = cJSON_GetObjectItem(root, "game");
	
	loadStarSystems(cJSON_GetObjectItem(game, "starSystems"));
	
	loadStats(cJSON_GetObjectItem(game, "stats"));
	
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
		
		loadMissions(starSystem, cJSON_GetObjectItem(starSystemJSON, "missions"));
	}
}

static void loadMissions(StarSystem *starSystem, cJSON *missionsCJSON)
{
	Mission *mission;
	cJSON *missionCJSON;
	
	for (missionCJSON = missionsCJSON->child ; missionCJSON != NULL ; missionCJSON = missionCJSON->next)
	{
		mission = getMission(starSystem, cJSON_GetObjectItem(missionCJSON, "filename")->valuestring);
		
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
	game.stats.missionsStarted = cJSON_GetObjectItem(stats, "missionsStarted")->valueint;
	game.stats.missionsCompleted = cJSON_GetObjectItem(stats, "missionsCompleted")->valueint;
	game.stats.shotsFired = cJSON_GetObjectItem(stats, "shotsFired")->valueint;
	game.stats.shotsHit = cJSON_GetObjectItem(stats, "shotsHit")->valueint;
	game.stats.missilesFired = cJSON_GetObjectItem(stats, "missilesFired")->valueint;
	game.stats.missilesHit = cJSON_GetObjectItem(stats, "missilesHit")->valueint;
	game.stats.enemiesKilled = cJSON_GetObjectItem(stats, "enemiesKilled")->valueint;
	game.stats.alliesKilled = cJSON_GetObjectItem(stats, "alliesKilled")->valueint;
	game.stats.playerKilled = cJSON_GetObjectItem(stats, "playerKilled")->valueint;
	game.stats.playerKills = cJSON_GetObjectItem(stats, "playerKills")->valueint;
	game.stats.time = cJSON_GetObjectItem(stats, "time")->valueint;
}
