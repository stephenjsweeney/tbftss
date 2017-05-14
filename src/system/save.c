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

#include "save.h"

static void saveStarSystems(cJSON *gameJSON);
static void saveChallenges(cJSON *gameJSON);
static cJSON *getMissionsJSON(StarSystem *starSystem);
static void saveStats(cJSON *gameJSON);
static void saveTrophies(cJSON *gameJSON);
static void saveFighterStats(cJSON *gameJSON);

void saveGame(void)
{
	char *out;
	cJSON *root, *gameJSON;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Saving Game ...");

	root = cJSON_CreateObject();
	gameJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "game", gameJSON);

	cJSON_AddStringToObject(gameJSON, "difficulty", getLookupName("DIFFICULTY_", game.difficulty));
	
	cJSON_AddStringToObject(gameJSON, "selectedStarSystem", game.selectedStarSystem);

	saveStarSystems(gameJSON);

	saveChallenges(gameJSON);

	saveStats(gameJSON);
	
	saveTrophies(gameJSON);
	
	saveFighterStats(gameJSON);

	out = cJSON_Print(root);

	writeFile(getSaveFilePath(SAVE_FILENAME), out);

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
		if (starSystem->totalMissions > 0)
		{
			starSystemJSON = cJSON_CreateObject();

			cJSON_AddStringToObject(starSystemJSON, "name", starSystem->name);
			cJSON_AddStringToObject(starSystemJSON, "side", getLookupName("SIDE_", starSystem->side));
			cJSON_AddItemToObject(starSystemJSON, "missions", getMissionsJSON(starSystem));

			cJSON_AddItemToArray(starSystemsJSON, starSystemJSON);
		}
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

		cJSON_AddItemToArray(missionsJSON, missionJSON);
	}

	return missionsJSON;
}

static void saveChallenges(cJSON *gameJSON)
{
	int i;
	Mission *mission;
	Challenge *c;
	cJSON *missionsJSON, *missionJSON, *challengesJSON, *challengeJSON;

	missionsJSON = cJSON_CreateArray();

	for (mission = game.challengeMissionHead.next ; mission != NULL ; mission = mission->next)
	{
		missionJSON = cJSON_CreateObject();

		cJSON_AddStringToObject(missionJSON, "filename", mission->filename);

		challengesJSON = cJSON_CreateArray();
		
		for (i = 0 ; i < MAX_CHALLENGES ; i++)
		{
			c = mission->challengeData.challenges[i];
			
			if (c)
			{
				challengeJSON = cJSON_CreateObject();
				cJSON_AddStringToObject(challengeJSON, "type", getLookupName("CHALLENGE_", c->type));
				cJSON_AddNumberToObject(challengeJSON, "value", c->value);
				cJSON_AddNumberToObject(challengeJSON, "passed", c->passed);
				
				cJSON_AddItemToArray(challengesJSON, challengeJSON);
			}
		}

		cJSON_AddItemToObject(missionJSON, "challenges", challengesJSON);

		cJSON_AddItemToArray(missionsJSON, missionJSON);
	}

	cJSON_AddItemToObject(gameJSON, "challenges", missionsJSON);
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

static void saveTrophies(cJSON *gameJSON)
{
	Trophy *t;
	cJSON *trophiesJSON, *trophyJSON;
	
	trophiesJSON = cJSON_CreateArray();

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (t->awarded)
		{
			trophyJSON = cJSON_CreateObject();
			
			cJSON_AddStringToObject(trophyJSON, "id", t->id);
			cJSON_AddNumberToObject(trophyJSON, "awardDate", t->awardDate);
			
			cJSON_AddItemToArray(trophiesJSON, trophyJSON);
		}
	}
	
	cJSON_AddItemToObject(gameJSON, "trophies", trophiesJSON);
}

static void saveFighterStats(cJSON *gameJSON)
{
	Tuple *t;
	cJSON *fighterStatsJSON, *fighterStatJSON;
	
	fighterStatsJSON = cJSON_CreateArray();

	for (t = game.fighterStatHead.next ; t != NULL ; t = t->next)
	{
		fighterStatJSON = cJSON_CreateObject();
		
		cJSON_AddStringToObject(fighterStatJSON, "key", t->key);
		cJSON_AddNumberToObject(fighterStatJSON, "value", t->value);
		
		cJSON_AddItemToArray(fighterStatsJSON, fighterStatJSON);
	}
	
	cJSON_AddItemToObject(gameJSON, "fighterStats", fighterStatsJSON);
}
