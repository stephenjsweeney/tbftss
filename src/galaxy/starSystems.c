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

#include "starSystems.h"

static void loadStarSystem(cJSON *starSystemJSON);
static void loadMissionMeta(char *filename, StarSystem *starSystem);

void initStarSystems(void)
{
	cJSON *root, *node;
	char *text;
	
	text = readFile("data/galaxy/starSystems.json");
	root = cJSON_Parse(text);
	
	for (node = cJSON_GetObjectItem(root, "starSystems")->child ; node != NULL ; node = node->next)
	{
		loadStarSystem(node);
	}
	
	cJSON_Delete(root);
	free(text);
}

static void loadStarSystem(cJSON *starSystemJSON)
{
	cJSON *node;
	StarSystem *starSystem;
	
	starSystem = malloc(sizeof(StarSystem));
	memset(starSystem, 0, sizeof(StarSystem));
	game.starSystemTail->next = starSystem;
	game.starSystemTail = starSystem;
	
	STRNCPY(starSystem->name, cJSON_GetObjectItem(starSystemJSON, "name")->valuestring, MAX_NAME_LENGTH);
	starSystem->side = lookup(cJSON_GetObjectItem(starSystemJSON, "side")->valuestring);
	starSystem->x = cJSON_GetObjectItem(starSystemJSON, "x")->valueint;
	starSystem->y = cJSON_GetObjectItem(starSystemJSON, "y")->valueint;
	
	if (cJSON_GetObjectItem(starSystemJSON, "fallsToPandorans"))
	{
		starSystem->fallsToPandorans = cJSON_GetObjectItem(starSystemJSON, "fallsToPandorans")->valueint;
	}
	
	starSystem->missionHead.completed = 1;
	starSystem->missionTail = &starSystem->missionHead;
	
	for (node = cJSON_GetObjectItem(starSystemJSON, "missions")->child ; node != NULL ; node = node->next)
	{
		loadMissionMeta(node->valuestring, starSystem);
	}
	
	starSystem->x *= 3;
	starSystem->y *= 3;
}

static void loadMissionMeta(char *filename, StarSystem *starSystem)
{
	Mission *mission;
	Challenge *challenge, *challengeTail;
	cJSON *root, *node;
	char *text;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);
	
	text = readFile(filename);
	
	root = cJSON_Parse(text);
	
	mission = malloc(sizeof(Mission));
	memset(mission, 0, sizeof(Mission));
	starSystem->missionTail->next = mission;
	starSystem->missionTail = mission;
	
	STRNCPY(mission->name, cJSON_GetObjectItem(root, "name")->valuestring, MAX_NAME_LENGTH);
	STRNCPY(mission->description, cJSON_GetObjectItem(root, "description")->valuestring, MAX_DESCRIPTION_LENGTH);
	STRNCPY(mission->filename, filename, MAX_DESCRIPTION_LENGTH);
	
	if (cJSON_GetObjectItem(root, "requires"))
	{
		STRNCPY(mission->requires, cJSON_GetObjectItem(root, "requires")->valuestring, MAX_DESCRIPTION_LENGTH);
	}
	
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
	
	challengeTail = &mission->challengeHead;
	
	node = cJSON_GetObjectItem(root, "challenges");
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			challenge = malloc(sizeof(Challenge));
			memset(challenge, 0, sizeof(Challenge));
			
			challenge->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			challenge->targetValue = cJSON_GetObjectItem(node, "targetValue")->valueint;
			
			challengeTail->next = challenge;
			challengeTail = challenge;
			
			node = node->next;
		}
	}
	
	cJSON_Delete(root);
	free(text);
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

void updateStarSystemDescriptions(void)
{
	StarSystem *starSystem;
	Mission *mission;
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		starSystem->completedMissions = starSystem->totalMissions;
		
		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			starSystem->totalMissions++;
			
			if (mission->completed)
			{
				starSystem->completedMissions++;
			}
		}
		
		sprintf(starSystem->description, "[ %s ]  [ Missions %d / %d ]", starSystem->name, starSystem->completedMissions, starSystem->totalMissions);
	}
}

void updateStarSystemMissions(void)
{
	StarSystem *starSystem;
	Mission *mission, *prev;
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		starSystem->completedMissions = starSystem->totalMissions = 0;
		
		prev = &starSystem->missionHead;
		
		for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
		{
			mission->available = isMissionAvailable(mission, prev);
			
			if (mission->available)
			{
				starSystem->totalMissions++;
				
				if (mission->completed)
				{
					starSystem->completedMissions++;
				}
			}
			
			prev = mission;
		}
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
