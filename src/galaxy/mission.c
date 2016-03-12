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

#include "mission.h"

static void loadObjectives(cJSON *node);
static void loadPlayer(cJSON *node);
static void loadFighters(cJSON *node);
static void loadCapitalShips(cJSON *node);
static void loadEntities(cJSON *node);
static void loadItems(cJSON *node);
static void loadLocations(cJSON *node);
static unsigned long hashcode(const char *str);
static void loadEpicData(cJSON *node);
static char *getAutoBackground(char *filename);
static char *getAutoPlanet(char *filename);
static char *getAutoMusic(char *filename);

Mission *loadMissionMeta(char *filename)
{
	int i;
	Mission *mission;
	Challenge *challenge;
	cJSON *root, *node;
	char *text;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);

	root = cJSON_Parse(text);

	mission = NULL;

	if (root)
	{
		mission = malloc(sizeof(Mission));
		memset(mission, 0, sizeof(Mission));

		STRNCPY(mission->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(mission->description, _(cJSON_GetObjectItem(root, "description")->valuestring), MAX_DESCRIPTION_LENGTH);
		STRNCPY(mission->filename, filename, MAX_DESCRIPTION_LENGTH);

		mission->requires = getJSONValue(root, "requires", 0);

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
			mission->challengeData.isChallenge = 1;

			/* limits */
			mission->challengeData.timeLimit = getJSONValue(node, "timeLimit", 0) * FPS;
			mission->challengeData.killLimit = getJSONValue(node, "killLimit", 0);
			mission->challengeData.escapeLimit = getJSONValue(node, "escapeLimit", 0);
			mission->challengeData.waypointLimit = getJSONValue(node, "waypointLimit", 0);
			mission->challengeData.itemLimit = getJSONValue(node, "itemLimit", 0);
			mission->challengeData.rescueLimit = getJSONValue(node, "rescueLimit", 0);

			/* restrictions */
			mission->challengeData.noMissiles = getJSONValue(node, "noMissiles", 0);
			mission->challengeData.noECM = getJSONValue(node, "noECM", 0);
			mission->challengeData.noBoost = getJSONValue(node, "noBoost", 0);
			mission->challengeData.noGuns = getJSONValue(node, "noGuns", 0);

			node = cJSON_GetObjectItem(node, "challenges");

			if (node)
			{
				node = node->child;

				i = 0;

				while (node && i < MAX_CHALLENGES)
				{
					challenge = malloc(sizeof(Challenge));
					memset(challenge, 0, sizeof(Challenge));

					challenge->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
					challenge->value = cJSON_GetObjectItem(node, "value")->valueint;

					mission->challengeData.challenges[i] = challenge;

					node = node->next;

					i++;
				}
			}
		}

		cJSON_Delete(root);
	}

	free(text);

	return mission;
}

void loadMission(char *filename)
{
	cJSON *root;
	char *text, music[MAX_DESCRIPTION_LENGTH], *background, *planet;
	float planetScale;

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

	if (cJSON_GetObjectItem(root, "epic"))
	{
		loadEpicData(cJSON_GetObjectItem(root, "epic"));
	}

	battle.manualComplete = getJSONValue(root, "manualComplete", 0);
	battle.unwinnable = getJSONValue(root, "unwinnable", 0);

	initScript(cJSON_GetObjectItem(root, "script"));

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
	planetScale = 75 + (rand() % 125);
	planetScale *= 0.01;
	battle.planetTexture = getTexture(planet);
	battle.planet.x = (SCREEN_WIDTH / 2) - (rand() % SCREEN_WIDTH) + (rand() % SCREEN_WIDTH);
	battle.planet.y = (SCREEN_HEIGHT / 2) - (rand() % SCREEN_HEIGHT) + (rand() % SCREEN_HEIGHT);
	SDL_QueryTexture(battle.planetTexture, NULL, NULL, &battle.planetWidth, &battle.planetHeight);
	battle.planetWidth *= planetScale;
	battle.planetHeight *= planetScale;

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

	countNumEnemies();

	initPlayer();

	initMissionInfo();

	addAllEntsToQuadtree();

	playMusic(music);
}

static char *getAutoBackground(char *filename)
{
	int hash;

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
	int hash;

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
	int hash;

	if (!game.currentMission->challengeData.isChallenge)
	{
		hash = hashcode(game.selectedStarSystem);
	}
	else
	{
		hash = hashcode(filename);
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

		completeConditions();

		retreatEnemies();

		player->flags |= EF_IMMORTAL;
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
	}
}

static void loadObjectives(cJSON *node)
{
	Objective *o;

	if (node)
	{
		node = node->child;

		while (node)
		{
			o = malloc(sizeof(Objective));
			memset(o, 0, sizeof(Objective));
			battle.objectiveTail->next = o;
			battle.objectiveTail = o;

			STRNCPY(o->description, _(cJSON_GetObjectItem(node, "description")->valuestring), MAX_DESCRIPTION_LENGTH);
			STRNCPY(o->targetName, cJSON_GetObjectItem(node, "targetName")->valuestring, MAX_NAME_LENGTH);
			o->targetValue = cJSON_GetObjectItem(node, "targetValue")->valueint;
			o->targetType = lookup(cJSON_GetObjectItem(node, "targetType")->valuestring);
			o->active = getJSONValue(node, "active", 1);
			o->isCondition = getJSONValue(node, "isCondition", 0);

			o->isEliminateAll = getJSONValue(node, "isEliminateAll", 0);
			if (o->isEliminateAll)
			{
				o->targetValue = 1;
			}

			node = node->next;
		}
	}
}

static void loadPlayer(cJSON *node)
{
	char *type;
	int side;

	type = cJSON_GetObjectItem(node, "type")->valuestring;
	side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);

	player = spawnFighter(type, 0, 0, side);
	player->x = BATTLE_AREA_WIDTH / 2;
	player->y = BATTLE_AREA_HEIGHT / 2;

	if (cJSON_GetObjectItem(node, "x"))
	{
		player->x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
		player->y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
	}

	if (strcmp(type, "Tug") == 0)
	{
		battle.stats[STAT_TUG]++;
	}

	if (strcmp(type, "Shuttle") == 0)
	{
		battle.stats[STAT_SHUTTLE]++;
	}
}

static void loadFighters(cJSON *node)
{
	Entity *e;
	char **types, *name, *groupName, *type;
	int side, scatter, number, active;
	int i, numTypes, addFlags, addAIFlags;
	long flags, aiFlags;
	float x, y;

	if (node)
	{
		node = node->child;

		while (node)
		{
			name = NULL;
			groupName = NULL;
			flags = -1;
			aiFlags = -1;

			types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			scatter = getJSONValue(node, "scatter", 1);
			active = getJSONValue(node, "active", 1);

			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			if (cJSON_GetObjectItem(node, "aiFlags"))
			{
				aiFlags = flagsToLong(cJSON_GetObjectItem(node, "aiFlags")->valuestring, &addAIFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				type = types[rand() % numTypes];

				e = spawnFighter(type, x, y, side);

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;

				if (flags != -1)
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

				if (aiFlags != -1)
				{
					if (addAIFlags)
					{
						e->aiFlags |= aiFlags;
					}
					else
					{
						e->aiFlags = aiFlags;

						SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "AI Flags for '%s' (%s) replaced", e->name, e->defName);
					}
				}

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}
			}

			node = node->next;

			for (i = 0 ; i < numTypes ; i++)
			{
				free(types[i]);
			}

			free(types);
		}
	}
}

static void loadCapitalShips(cJSON *node)
{
	Entity *e;
	char **types, *name, *groupName, *type;
	int side, scatter, number, active;
	int i, numTypes, addFlags;
	long flags;
	float x, y;

	if (node)
	{
		node = node->child;

		while (node)
		{
			name = NULL;
			groupName = NULL;
			flags = -1;

			types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			scatter = getJSONValue(node, "scatter", 1);
			active = getJSONValue(node, "active", 1);

			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				type = types[rand() % numTypes];

				e = spawnCapitalShip(type, x, y, side);

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}

				if (flags != -1)
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

				updateCapitalShipComponentProperties(e);
			}

			node = node->next;

			for (i = 0 ; i < numTypes ; i++)
			{
				free(types[i]);
			}

			free(types);
		}
	}
}

static void loadEntities(cJSON *node)
{
	Entity *e;
	char *name, *groupName;
	int i, type, scatter, number, active, addFlags;
	float x, y;
	long flags;

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

			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			active = getJSONValue(node, "active", 1);
			scatter = getJSONValue(node, "scatter", 1);
			
			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				switch (type)
				{
					case ET_WAYPOINT:
						e = spawnWaypoint();
						break;

					case ET_JUMPGATE:
						e = spawnJumpgate();
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
				
				if (flags != -1)
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

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				e->active = active;

				SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			}

			node = node->next;
		}
	}
}

static void loadItems(cJSON *node)
{
	Entity *e;
	char *name, *groupName, *type;
	int i, scatter, number, active, addFlags;
	long flags;
	float x, y;

	flags = -1;
	scatter = 1;

	if (node)
	{
		node = node->child;

		while (node)
		{
			type = cJSON_GetObjectItem(node, "type")->valuestring;
			x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
			name = NULL;
			groupName = NULL;

			name = getJSONValueStr(node, "name", NULL);
			groupName = getJSONValueStr(node, "groupName", NULL);
			number = getJSONValue(node, "number", 1);
			scatter = getJSONValue(node, "scatter", 1);
			active = getJSONValue(node, "active", 1);

			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}

			for (i = 0 ; i < number ; i++)
			{
				e = spawnItem(type);

				if (name)
				{
					STRNCPY(e->name, name, MAX_NAME_LENGTH);
				}

				if (groupName)
				{
					STRNCPY(e->groupName, groupName, MAX_NAME_LENGTH);
				}

				if (flags != -1)
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
				e->active = active;

				if (scatter > 1)
				{
					e->x += (rand() % scatter) - (rand() % scatter);
					e->y += (rand() % scatter) - (rand() % scatter);
				}

				SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			}

			node = node->next;
		}
	}
}

static void loadLocations(cJSON *node)
{
	int active;
	Location *l;

	if (node)
	{
		node = node->child;

		while (node)
		{
			l = malloc(sizeof(Location));
			memset(l, 0, sizeof(Location));
			battle.locationTail->next = l;
			battle.locationTail = l;

			STRNCPY(l->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			l->x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
			l->y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;

			l->size = cJSON_GetObjectItem(node, "size")->valueint;

			active = getJSONValue(node, "active", 1);

			l->x += (SCREEN_WIDTH / 2);
			l->y += (SCREEN_HEIGHT / 2);
			l->active = active;

			node = node->next;
		}
	}
}

static void loadEpicData(cJSON *node)
{
	Entity *e;
	int numFighters[SIDE_MAX];
	memset(numFighters, 0, sizeof(int) * SIDE_MAX);

	battle.isEpic = 1;

	battle.epicFighterLimit = cJSON_GetObjectItem(node, "fighterLimit")->valueint;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
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
	Mission *mission;

	/* First, search the star systems */
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			if (strcmp(mission->filename, filename) == 0)
			{
				return mission;
			}
		}
	}

	/* now search the challenges */
	for (mission = game.challengeMissionHead.next ; mission != NULL ; mission = mission->next)
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
	return prev->completed && mission->requires <= game.completedMissions;
}

static unsigned long hashcode(const char *str)
{
    unsigned long hash = 5381;
    int c;

	c = *str;

	while (c)
	{
        hash = ((hash << 5) + hash) + c;

        c = *str++;
	}

	return abs(hash);
}
