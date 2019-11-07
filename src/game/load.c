/*
Copyright (C) 2015-2019 Parallel Realities

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

static void loadStats(cJSON *statsJSON);
static void loadStarSystems(cJSON *starSystemsJSON);
static void loadMissions(cJSON *missionsCJSON);
static void loadChallenges(cJSON *challengesCJSON);
static void loadTrophies(cJSON *trophiesJSON);
static void loadFighterStats(cJSON *fighterStatsJSON);

void loadGame(void)
{
	cJSON *root, *gameJSON;
	char *text;

	text = readFile(getSaveFilePath(SAVE_FILENAME));
	root = cJSON_Parse(text);

	gameJSON = cJSON_GetObjectItem(root, "game");

	STRNCPY(game.selectedStarSystem, cJSON_GetObjectItem(gameJSON, "selectedStarSystem")->valuestring, MAX_NAME_LENGTH);

	loadStarSystems(cJSON_GetObjectItem(gameJSON, "starSystems"));

	loadChallenges(cJSON_GetObjectItem(gameJSON, "challenges"));

	loadStats(cJSON_GetObjectItem(gameJSON, "stats"));

	loadTrophies(cJSON_GetObjectItem(gameJSON, "trophies"));

	loadFighterStats(cJSON_GetObjectItem(gameJSON, "fighterStats"));

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

static void loadMissions(cJSON *missionsJSON)
{
	Mission *mission;
	cJSON *missionJSON;

	for (missionJSON = missionsJSON->child ; missionJSON != NULL ; missionJSON = missionJSON->next)
	{
		mission = getMission(cJSON_GetObjectItem(missionJSON, "filename")->valuestring);

		if (mission)
		{
			mission->completed = cJSON_GetObjectItem(missionJSON, "completed")->valueint;
		}
	}
}

static void loadChallenges(cJSON *missionsJSON)
{
	Mission *mission;
	Challenge *challenge;
	cJSON *missionJSON, *challengeJSON;
	int type, value;

	if (missionsJSON)
	{
		for (missionJSON = missionsJSON->child ; missionJSON != NULL ; missionJSON = missionJSON->next)
		{
			mission = getMission(cJSON_GetObjectItem(missionJSON, "filename")->valuestring);

			for (challengeJSON = cJSON_GetObjectItem(missionJSON, "challenges")->child ; challengeJSON != NULL ; challengeJSON = challengeJSON->next)
			{
				type = lookup(cJSON_GetObjectItem(challengeJSON, "type")->valuestring);
				value = cJSON_GetObjectItem(challengeJSON, "value")->valueint;

				challenge = getChallenge(mission, type, value);

				if (challenge)
				{
					challenge->passed = cJSON_GetObjectItem(challengeJSON, "passed")->valueint;
				}
			}
		}
	}
}

static void loadStats(cJSON *statsJSON)
{
	int i;
	char *statName;

	for (i = 0 ; i < STAT_MAX ; i++)
	{
		statName = getLookupName("STAT_", i);

		if (statName && cJSON_GetObjectItem(statsJSON, statName))
		{
			game.stats[i] = cJSON_GetObjectItem(statsJSON, statName)->valueint;
		}
	}
}

static void loadTrophies(cJSON *trophiesJSON)
{
	Trophy *t;
	cJSON *trophyJSON;

	if (trophiesJSON)
	{
		for (trophyJSON = trophiesJSON->child ; trophyJSON != NULL ; trophyJSON = trophyJSON->next)
		{
			t = getTrophy(cJSON_GetObjectItem(trophyJSON, "id")->valuestring);

			if (t)
			{
				t->awardDate = cJSON_GetObjectItem(trophyJSON, "awardDate")->valueint;
				t->awarded = 1;
			}
		}
	}
}

static void loadFighterStats(cJSON *fighterStatsJSON)
{
	Tuple *t, *tail;
	cJSON *fighterStatJSON;

	destroyFighterStats();

	tail = &game.fighterStatHead;

	if (fighterStatsJSON)
	{
		for (fighterStatJSON = fighterStatsJSON->child ; fighterStatJSON != NULL ; fighterStatJSON = fighterStatJSON->next)
		{
			t = malloc(sizeof(Tuple));
			memset(t, 0, sizeof(Tuple));

			STRNCPY(t->key, cJSON_GetObjectItem(fighterStatJSON, "key")->valuestring, MAX_NAME_LENGTH);
			t->value = cJSON_GetObjectItem(fighterStatJSON, "value")->valueint;

			tail->next = t;
			tail = t;
		}
	}
}
