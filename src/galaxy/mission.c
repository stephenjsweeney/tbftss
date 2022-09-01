/*
Copyright (C) 2015-2019,2022 Parallel Realities

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

#include <time.h>

#include "../common.h"

#include "../battle/capitalShips.h"
#include "../battle/entities.h"
#include "../battle/fighters.h"
#include "../battle/items.h"
#include "../battle/jumpgate.h"
#include "../battle/locations.h"
#include "../battle/mine.h"
#include "../battle/missionInfo.h"
#include "../battle/objectives.h"
#include "../battle/player.h"
#include "../battle/script.h"
#include "../battle/spawners.h"
#include "../battle/waypoints.h"
#include "../challenges/challenges.h"
#include "../galaxy/starSystems.h"
#include "../game/trophies.h"
#include "../json/cJSON.h"
#include "../system/atlas.h"
#include "../system/io.h"
#include "../system/lookup.h"
#include "../system/resources.h"
#include "../system/sound.h"
#include "../system/textures.h"
#include "../system/transition.h"
#include "../system/util.h"
#include "../system/widgets.h"
#include "mission.h"

extern App     app;
extern Battle  battle;
extern Dev     dev;
extern Entity *player;
extern Game    game;

static void  loadEntities(cJSON *node);
static void  loadEpicData(cJSON *node);
static char *getAutoBackground(char *filename);
static char *getAutoPlanet(char *filename);
static char *getAutoMusic(char *filename);

Mission *loadMissionMeta(char *filename)
{
	Mission *mission;
	cJSON   *root, *node;
	char    *text;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);

	root = cJSON_Parse(text);

	mission = NULL;

	if (root)
	{
		mission = malloc(sizeof(Mission));
		memset(mission, 0, sizeof(Mission));

		STRNCPY(mission->name, _(cJSON_GetObjectItem(root, "name")->valuestring), MAX_NAME_LENGTH);
		STRNCPY(mission->description, _(cJSON_GetObjectItem(root, "description")->valuestring), MAX_DESCRIPTION_LENGTH);
		STRNCPY(mission->filename, filename, MAX_DESCRIPTION_LENGTH);

		mission->requires = getJSONValue(root, "requires", 0);

		mission->isOptional = getJSONValue(root, "isOptional", 0);
		mission->requiresOptional = getJSONValue(root, "requiresOptional", 0);
		mission->expires = getJSONValue(root, "expires", 0);

		if (cJSON_GetObjectItem(root, "epic"))
		{
			mission->epic = 1;
		}

		node = cJSON_GetObjectItem(root, "player");

		if (node)
		{
			STRNCPY(mission->pilot, cJSON_GetObjectItem(node, "pilot")->valuestring, MAX_NAME_LENGTH);
			STRNCPY(mission->squadron, cJSON_GetObjectItem(node, "squadron")->valuestring, MAX_NAME_LENGTH);
			STRNCPY(mission->craft, cJSON_GetObjectItem(node, "type")->valuestring, MAX_NAME_LENGTH);
		}

		node = cJSON_GetObjectItem(root, "challenge");

		if (node)
		{
			loadChallenge(mission, node);
		}

		cJSON_Delete(root);
	}
	else
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Failed to load '%s'", filename);
		exit(1);
	}

	free(text);

	return mission;
}

void loadMission(char *filename)
{
	cJSON *root;
	char  *text, music[MAX_DESCRIPTION_LENGTH], *background, *planet;
	float  planetScale;

	startSectionTransition();

	stopMusic();

	text = readFile(filename);

	root = cJSON_Parse(text);

	srand(hashcode(filename));

	loadObjectives(cJSON_GetObjectItem(root, "objectives"));

	loadPlayer(cJSON_GetObjectItem(root, "player"));

	loadFighters(cJSON_GetObjectItem(root, "fighters"));

	loadCapitalShips(cJSON_GetObjectItem(root, "capitalShips"));

	loadEntities(cJSON_GetObjectItem(root, "entities"));

	loadItems(cJSON_GetObjectItem(root, "items"));

	loadLocations(cJSON_GetObjectItem(root, "locations"));

	loadSpawners(cJSON_GetObjectItem(root, "spawners"));

	if (cJSON_GetObjectItem(root, "epic"))
	{
		loadEpicData(cJSON_GetObjectItem(root, "epic"));
	}

	battle.manualComplete = getJSONValue(root, "manualComplete", 0);
	battle.unwinnable = getJSONValue(root, "unwinnable", 0);
	battle.waypointAutoAdvance = getJSONValue(root, "waypointAutoAdvance", 0);
	battle.hasSuspicionLevel = getJSONValue(root, "hasSuspicionLevel", 0);

	initScript(root);

	/* music, planet, and background loading must come last, so AUTO works properly */

	STRNCPY(music, cJSON_GetObjectItem(root, "music")->valuestring, MAX_DESCRIPTION_LENGTH);
	if (strcmp(music, "AUTO") == 0)
	{
		STRNCPY(music, getAutoMusic(filename), MAX_DESCRIPTION_LENGTH);
	}

	background = cJSON_GetObjectItem(root, "background")->valuestring;
	if (strcmp(background, "AUTO") == 0)
	{
		background = getAutoBackground(filename);
	}
	battle.background = getTexture(background);

	planet = cJSON_GetObjectItem(root, "planet")->valuestring;
	if (strcmp(planet, "AUTO") == 0)
	{
		planet = getAutoPlanet(filename);
	}

	battle.planetTexture = getAtlasImage(planet);
	battle.fireStormTexture = getAtlasImage("gfx/misc/torelliFireStorm.png");
	battle.planet.x = (app.winWidth / 2) - (rand() % app.winWidth) + (rand() % app.winWidth);
	battle.planet.y = (app.winHeight / 2) - (rand() % app.winHeight) + (rand() % app.winHeight);

	if (strcmp(planet, "gfx/planets/star.png") != 0)
	{
		battle.planetWidth = battle.planetTexture->rect.w;
		battle.planetHeight = battle.planetTexture->rect.h;

		planetScale = 75 + (rand() % 125);
		planetScale *= 0.01;

		if (getJSONValue(root, "largePlanet", 0))
		{
			battle.planet.x = (app.winWidth / 2);
			battle.planet.y = (app.winHeight / 2);
			planetScale = 5;
		}

		battle.planetWidth *= planetScale;
		battle.planetHeight *= planetScale;
	}

	srand(time(NULL));

	free(text);

	endSectionTransition();

	/* only increment num missions / challenges started if there are some (Free Flight excluded, for example) */
	if (battle.objectiveHead.next)
	{
		game.stats[STAT_MISSIONS_STARTED]++;
	}
	else if (game.currentMission->challengeData.isChallenge)
	{
		game.stats[STAT_CHALLENGES_STARTED]++;
	}
	else
	{
		battle.status = MS_IN_PROGRESS;
	}

	if (battle.waypointAutoAdvance)
	{
		activateNextWaypoint();
	}

	countNumEnemies();

	initPlayer();

	initMissionInfo();

	setInitialPlayerAngle();

	addAllToQuadtree();

	playMusic(music, 1);
}

static char *getAutoBackground(char *filename)
{
	unsigned long hash;

	if (!game.currentMission->challengeData.isChallenge)
	{
		hash = hashcode(game.selectedStarSystem);
	}
	else
	{
		hash = hashcode(filename);
	}

	return getBackgroundTextureName(hash);
}

static char *getAutoPlanet(char *filename)
{
	unsigned long hash;

	if (!game.currentMission->challengeData.isChallenge)
	{
		hash = hashcode(game.selectedStarSystem);
	}
	else
	{
		hash = hashcode(filename);
	}

	return getPlanetTextureName(hash);
}

static char *getAutoMusic(char *filename)
{
	unsigned long hash;

	if (!game.currentMission->challengeData.isChallenge)
	{
		hash = hashcode(game.selectedStarSystem);
	}
	else
	{
		hash = hashcode(game.currentMission->description);
	}

	return getMusicFilename(hash);
}

void completeMission(void)
{
	if (battle.status == MS_IN_PROGRESS)
	{
		battle.status = MS_COMPLETE;
		battle.missionFinishedTimer = FPS;
		selectWidget("continue", "battleWon");

		game.stats[STAT_MISSIONS_COMPLETED]++;

		if (game.currentMission->isOptional)
		{
			game.stats[STAT_OPTIONAL_COMPLETED]++;
		}

		completeConditions();

		retreatEnemies();

		player->flags |= EF_IMMORTAL;

		awardStatsTrophies();

		awardPostMissionTrophies();

		awardCraftTrophy();
	}
}

void failMission(void)
{
	if (battle.status == MS_IN_PROGRESS)
	{
		battle.status = MS_FAILED;
		battle.missionFinishedTimer = FPS;
		selectWidget("retry", "battleLost");

		failIncompleteObjectives();

		player->flags |= EF_IMMORTAL;

		awardStatsTrophies();
	}
}

static void loadEntities(cJSON *node)
{
	Entity *e;
	char   *name, *groupName;
	int     i, type, scatter, number, active, addFlags, side;
	float   x, y;
	long    flags;

	if (node)
	{
		node = node->child;

		while (node)
		{
			e = NULL;
			type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			name = NULL;
			groupName = NULL;
			flags = -1;

			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			active = getJSONValue(node, "active", 1);
			scatter = getJSONValue(node, "scatter", 1);
			side = lookup(getJSONValueStr(node, "side", "SIDE_NONE"));

			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			for (i = 0; i < number; i++)
			{
				switch (type)
				{
					case ET_WAYPOINT:
						e = spawnWaypoint();
						active = 0;
						break;

					case ET_JUMPGATE:
						e = spawnJumpgate(side, flags);
						break;

					case ET_MINE:
					case ET_SHADOW_MINE:
						e = spawnMine(type);
						break;

					default:
						printf("Error: Unhandled entity type: %s\n", cJSON_GetObjectItem(node, "type")->valuestring);
						exit(1);
						break;
				}

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}

				if (type != ET_JUMPGATE && flags != -1)
				{
					if (addFlags)
					{
						e->flags |= flags;
					}
					else
					{
						e->flags = flags;

						SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Flags for '%s' (%s) replaced", e->name, e->defName);
					}
				}

				e->x = x;
				e->y = y;

				e->side = side;

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;
			}

			node = node->next;
		}
	}
}

static void loadEpicData(cJSON *node)
{
	Entity *e;
	int     numFighters[SIDE_MAX];
	memset(numFighters, 0, sizeof(int) * SIDE_MAX);

	battle.isEpic = 1;

	battle.epicFighterLimit = cJSON_GetObjectItem(node, "fighterLimit")->valueint;
	battle.unlimitedEnemies = getJSONValue(node, "unlimitedEnemies", 0);
	battle.epicLives = getJSONValue(node, "lives", 0);
	battle.epicKills = getJSONValue(node, "kills", 0);

	if (battle.epicLives > 0)
	{
		addEpicLivesObjective();
	}

	if (battle.epicKills != 0)
	{
		addEpicKillsObjective();
	}

	for (e = battle.entityHead.next; e != NULL; e = e->next)
	{
		if (e->active && e->type == ET_FIGHTER && numFighters[e->side]++ >= battle.epicFighterLimit)
		{
			e->active = 0;
		}
	}
}

Mission *getMission(char *filename)
{
	StarSystem *starSystem;
	Mission    *mission;

	/* First, search the star systems */
	for (starSystem = game.starSystemHead.next; starSystem != NULL; starSystem = starSystem->next)
	{
		for (mission = starSystem->missionHead.next; mission != NULL; mission = mission->next)
		{
			if (strcmp(mission->filename, filename) == 0)
			{
				return mission;
			}
		}
	}

	/* now search the challenges */
	for (mission = game.challengeMissionHead.next; mission != NULL; mission = mission->next)
	{
		if (strcmp(mission->filename, filename) == 0)
		{
			return mission;
		}
	}

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "No such mission '%s'", filename);

	return NULL;
}

void updateAllMissions(void)
{
	updateStarSystemMissions();

	updateChallengeMissions();
}

int isMissionAvailable(Mission *mission, Mission *prev)
{
	if (!mission->isOptional)
	{
		return (prev->completed && game.completedMissions >= mission->requires) || dev.debug;
	}
	else
	{
		return mission->completed || (game.completedMissions >= mission->requires && game.stats[STAT_OPTIONAL_COMPLETED] >= mission->requiresOptional && game.completedMissions < mission->expires) || dev.debug;
	}
}
