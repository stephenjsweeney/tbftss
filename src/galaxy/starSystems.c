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

#include "starSystems.h"

static void loadMissions(StarSystem *starSystem);
static StarSystem *loadStarSystem(cJSON *starSystemJSON);

void initStarSystems(void)
{
	cJSON *root, *node;
	char *text;
	StarSystem *starSystem, *tail;

	tail = &game.starSystemHead;

	text = readFile("data/galaxy/starSystems.json");
	root = cJSON_Parse(text);

	for (node = cJSON_GetObjectItem(root, "starSystems")->child ; node != NULL ; node = node->next)
	{
		starSystem = loadStarSystem(node);
		tail->next = starSystem;
		tail = starSystem;
	}

	cJSON_Delete(root);
	free(text);
}

static StarSystem *loadStarSystem(cJSON *starSystemJSON)
{
	StarSystem *starSystem;

	starSystem = malloc(sizeof(StarSystem));
	memset(starSystem, 0, sizeof(StarSystem));

	STRNCPY(starSystem->name, cJSON_GetObjectItem(starSystemJSON, "name")->valuestring, MAX_NAME_LENGTH);
	starSystem->side = lookup(cJSON_GetObjectItem(starSystemJSON, "side")->valuestring);
	starSystem->x = cJSON_GetObjectItem(starSystemJSON, "x")->valueint;
	starSystem->y = cJSON_GetObjectItem(starSystemJSON, "y")->valueint;
	starSystem->fallsToPandorans = getJSONValue(starSystemJSON, "fallsToPandorans", 0);
	
	starSystem->type = (starSystem->side != SIDE_PANDORAN) ? SS_NORMAL : SS_PANDORAN;

	if (strcmp(starSystem->name, "Sol") == 0)
	{
		starSystem->type = SS_SOL;
	}

	starSystem->missionHead.completed = 1;

	loadMissions(starSystem);

	starSystem->x *= 3;
	starSystem->y *= 3;

	return starSystem;
}

static void loadMissions(StarSystem *starSystem)
{
	int i, count;
	char name[MAX_NAME_LENGTH];
	char path[MAX_FILENAME_LENGTH];
	char **filenames;
	Mission *mission, *tail;

	tail = &starSystem->missionHead;

	STRNCPY(name, starSystem->name, MAX_NAME_LENGTH);

	for (i = 0 ; name[i] ; i++)
	{
		name[i] = tolower(name[i]);
	}
	
	sprintf(path, "data/missions/%s", name);
	
	filenames = getFileList(path, &count);
	
	for (i = 0 ; i < count ; i++)
	{
		sprintf(path, "data/missions/%s/%s", name, filenames[i]);
		
		mission = loadMissionMeta(path);
		
		if (mission)
		{
			tail->next = mission;
			tail = mission;
		}
		
		free(filenames[i]);
	}
	
	free(filenames);
}

StarSystem *getStarSystem(char *name)
{
	StarSystem *starSystem;

	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		if (strcmp(starSystem->name, name) == 0)
		{
			return starSystem;
		}
	}

	return NULL;
}

void updateStarSystemMissions(void)
{
	StarSystem *starSystem;
	Mission *mission, *prev;
	int numOptional, completedOptional;

	game.completedMissions = game.totalMissions = game.availableMissions = 0;

	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		numOptional = completedOptional = starSystem->completedMissions = starSystem->availableMissions = starSystem->totalMissions = 0;

		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			starSystem->totalMissions++;

			if (mission->completed)
			{
				starSystem->completedMissions++;
			}
			
			if (mission->isOptional && starSystem->type == SS_NORMAL)
			{
				numOptional++;
				
				if (mission->completed)
				{
					completedOptional++;
				}
			}
		}

		if (starSystem->type == SS_NORMAL)
		{
			game.totalMissions += (starSystem->totalMissions - numOptional);
			game.completedMissions += (starSystem->completedMissions - completedOptional);
		}
	}

	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		prev = &starSystem->missionHead;

		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			mission->available = starSystem->type == SS_SOL || isMissionAvailable(mission, prev);

			if (mission->available)
			{
				starSystem->availableMissions++;
				
				starSystem->activeMission = mission;
			}

			prev = mission;
		}

		if (starSystem->type == SS_NORMAL)
		{
			game.availableMissions += starSystem->availableMissions;
		}

		sprintf(starSystem->description, "[ %s ]  [ Missions %d / %d ]", starSystem->name, starSystem->completedMissions, starSystem->availableMissions);
	}
}

void destroyStarSystems(void)
{
	StarSystem *starSystem;
	Mission *mission;

	while (game.starSystemHead.next)
	{
		starSystem = game.starSystemHead.next;

		while (starSystem->missionHead.next)
		{
			mission = starSystem->missionHead.next;
			starSystem->missionHead.next = mission->next;
			free(mission);
		}

		game.starSystemHead.next = starSystem->next;
		free(starSystem);
	}
}
