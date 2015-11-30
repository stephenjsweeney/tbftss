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

#include "save.h"

static void saveStarSystems(cJSON *gameJSON);
static cJSON *getMissionsJSON(StarSystem *starSystem);
static cJSON *getChallengesJSON(Mission *mission);
static void saveStats(cJSON *gameJSON);

void saveGame(void)
{
	char *out;
	cJSON *root, *gameJSON;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Saving Game ...");

	root = cJSON_CreateObject();
	gameJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "game", gameJSON);
	
	cJSON_AddStringToObject(gameJSON, "selectedStarSystem", game.selectedStarSystem);
	
	saveStarSystems(gameJSON);

	saveStats(gameJSON);

	out = cJSON_Print(root);

	writeFile(getSaveFilePath("game.save"), out);

	cJSON_Delete(root);
	free(out);
}

static void saveStarSystems(cJSON *gameJSON)
{
	cJSON *starSystemJSON, *starSystemsJSON;
	StarSystem *starSystem;
	
	starSystemsJSON = cJSON_CreateArray();

	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		starSystemJSON = cJSON_CreateObject();
		
		cJSON_AddStringToObject(starSystemJSON, "name", starSystem->name);
		cJSON_AddStringToObject(starSystemJSON, "side", getLookupName("SIDE_", starSystem->side));
		cJSON_AddItemToObject(starSystemJSON, "missions", getMissionsJSON(starSystem));
		
		cJSON_AddItemToArray(starSystemsJSON, starSystemJSON);
	}
	
	cJSON_AddItemToObject(gameJSON, "starSystems", starSystemsJSON);
}

static cJSON *getMissionsJSON(StarSystem *starSystem)
{
	cJSON *missionJSON, *missionsJSON;
	Mission *mission;
	
	missionsJSON = cJSON_CreateArray();
	
	for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
	{
		missionJSON = cJSON_CreateObject();
		
		cJSON_AddStringToObject(missionJSON, "filename", mission->filename);
		cJSON_AddNumberToObject(missionJSON, "completed", mission->completed);
		
		if (mission->challengeHead.next)
		{
			cJSON_AddItemToObject(missionJSON, "challenges", getChallengesJSON(mission));
		}
		
		cJSON_AddItemToArray(missionsJSON, missionJSON);
	}
	
	return missionsJSON;
}

static cJSON *getChallengesJSON(Mission *mission)
{
	cJSON *challengesJSON, *challengeJSON;
	Challenge *challenge;
	
	challengesJSON = cJSON_CreateArray();
	
	for (challenge = mission->challengeHead.next ; challenge != NULL ; challenge = challenge->next)
	{
		challengeJSON = cJSON_CreateObject();
		
		cJSON_AddStringToObject(challengeJSON, "type", getLookupName("CHALLENGE_", challenge->type));
		cJSON_AddNumberToObject(challengeJSON, "targetValue", challenge->targetValue);
		cJSON_AddNumberToObject(challengeJSON, "passed", challenge->passed);
		
		cJSON_AddItemToArray(challengesJSON, challengeJSON);
	}
	
	return challengesJSON;
}

static void saveStats(cJSON *gameJSON)
{
	int i;
	
	cJSON *stats = cJSON_CreateObject();
	
	for (i = 0 ; i < STAT_MAX ; i++)
	{
		cJSON_AddNumberToObject(stats, getLookupName("STAT_", i), game.stats[i]);
	}

	cJSON_AddItemToObject(gameJSON, "stats", stats);
}
