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

#include "mission.h"

static void loadObjectives(cJSON *node);
static void loadPlayer(cJSON *node);
static void loadFighters(cJSON *node);
static void loadCapitalShips(cJSON *node);
static void loadEntities(cJSON *node);
static void loadItems(cJSON *node);
static unsigned long hashcode(const char *str);
static char **toTypeArray(char *types, int *numTypes);
static void loadEpicData(cJSON *node);

void loadMission(char *filename)
{
	cJSON *root;
	char *text, music[MAX_NAME_LENGTH];
	
	startSectionTransition();
	
	stopMusic();
	
	text = readFile(getFileLocation(filename));
	
	srand(hashcode(filename));
	
	root = cJSON_Parse(text);
	
	battle.background = getTexture(cJSON_GetObjectItem(root, "background")->valuestring);
	battle.planetTexture = getTexture(cJSON_GetObjectItem(root, "planet")->valuestring);
	battle.planet.x = ((200 + rand() % 100) / 10) * GRID_CELL_WIDTH;
	battle.planet.y = ((200 + rand() % 100) / 10) * GRID_CELL_HEIGHT;
	
	loadObjectives(cJSON_GetObjectItem(root, "objectives"));
		
	loadPlayer(cJSON_GetObjectItem(root, "player"));
	
	loadFighters(cJSON_GetObjectItem(root, "fighters"));
	
	loadCapitalShips(cJSON_GetObjectItem(root, "capitalShips"));
	
	loadEntities(cJSON_GetObjectItem(root, "entities"));
	
	loadItems(cJSON_GetObjectItem(root, "items"));
	
	STRNCPY(music, cJSON_GetObjectItem(root, "music")->valuestring, MAX_NAME_LENGTH);
	
	if (cJSON_GetObjectItem(root, "epic"))
	{
		loadEpicData(cJSON_GetObjectItem(root, "epic"));
	}
	
	initScript(cJSON_GetObjectItem(root, "script"));
	
	free(text);
	
	srand(time(NULL));
	
	endSectionTransition();
	
	/* only increment num missions started if there are objectives (Free Flight excluded, for example) */
	if (battle.objectiveHead.next)
	{
		game.stats[STAT_MISSIONS_STARTED]++;
	}
	else 
	{
		battle.status = MS_IN_PROGRESS;
	}
	
	activateNextWaypoint();
	
	initPlayer();
	
	playMusic(music);
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
			
			o->active = 1;
			STRNCPY(o->description, cJSON_GetObjectItem(node, "description")->valuestring, MAX_DESCRIPTION_LENGTH);
			STRNCPY(o->targetName, cJSON_GetObjectItem(node, "targetName")->valuestring, MAX_NAME_LENGTH);
			o->targetValue = cJSON_GetObjectItem(node, "targetValue")->valueint;
			o->targetType = lookup(cJSON_GetObjectItem(node, "targetType")->valuestring);
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				o->active = cJSON_GetObjectItem(node, "active")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "isCondition"))
			{
				o->isCondition = cJSON_GetObjectItem(node, "isCondition")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "isOptional"))
			{
				o->isOptional = cJSON_GetObjectItem(node, "isOptional")->valueint;
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
	player->x = (GRID_SIZE * GRID_CELL_WIDTH) / 2;
	player->y = (GRID_SIZE * GRID_CELL_HEIGHT) / 2;
	
	if (cJSON_GetObjectItem(node, "x"))
	{
		player->x = (cJSON_GetObjectItem(node, "x")->valueint * GRID_CELL_WIDTH);
		player->y = (cJSON_GetObjectItem(node, "y")->valueint * GRID_CELL_HEIGHT);
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
			scatter = 1;
			active = 1;
			number = 1;
			
			types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "number"))
			{
				number = cJSON_GetObjectItem(node, "number")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "scatter"))
			{
				scatter = cJSON_GetObjectItem(node, "scatter")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				active = cJSON_GetObjectItem(node, "active")->valueint;
			}
			
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
			scatter = 1;
			active = 1;
			number = 1;
			flags = -1;
			
			types = toTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "number"))
			{
				number = cJSON_GetObjectItem(node, "number")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "scatter"))
			{
				scatter = cJSON_GetObjectItem(node, "scatter")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				active = cJSON_GetObjectItem(node, "active")->valueint;
			}
			
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
				
				updateCapitalShipComponentNames(e);
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
	int i, type, scatter, number, active;
	float x, y;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			e = NULL;
			type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			name = NULL;
			groupName = NULL;
			number = 1;
			active = 1;
			scatter = 1;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "number"))
			{
				number = cJSON_GetObjectItem(node, "number")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				active = cJSON_GetObjectItem(node, "active")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "scatter"))
			{
				scatter = cJSON_GetObjectItem(node, "scatter")->valueint;
			}
			
			for (i = 0 ; i < number ; i++)
			{
				switch (type)
				{
					case ET_WAYPOINT:
						e = spawnWaypoint();
						break;
						
					case ET_EXTRACTION_POINT:
						e = spawnExtractionPoint();
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
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			name = NULL;
			groupName = NULL;
			number = 1;
			active = 1;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "number"))
			{
				number = cJSON_GetObjectItem(node, "number")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "scatter"))
			{
				scatter = cJSON_GetObjectItem(node, "scatter")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
			}
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				active = cJSON_GetObjectItem(node, "active")->valueint;
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

static char **toTypeArray(char *types, int *numTypes)
{
	int i;
	char **typeArray, *type;
	
	*numTypes = 1;
	
	for (i = 0 ; i < strlen(types) ; i++)
	{
		if (types[i] == ';')
		{
			*numTypes = *numTypes + 1;
		}
	}

	typeArray = malloc(*numTypes * sizeof(char*));
	
	i = 0;
	type = strtok(types, ";");
	while (type)
	{
		typeArray[i] = malloc(strlen(type) + 1);
		strcpy(typeArray[i], type);
		
		type = strtok(NULL, ";");
		
		i++;
	}
	
	return typeArray;
}

static void loadEpicData(cJSON *node)
{
	Entity *e;
	int numFighters[SIDE_MAX];
	memset(numFighters, 0, sizeof(int) * SIDE_MAX);
	
	battle.epic = 1;
	
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
	
	return NULL;
}

int isMissionAvailable(Mission *mission, Mission *prev)
{
	Mission *reqMission;
	
	if (!dev.debug)
	{
		if (mission->requires)
		{
			if (strcmp(mission->requires, "PREVIOUS") == 0)
			{
				return prev->completed;
			}
			else
			{
				reqMission = getMission(mission->requires);
					
				if (reqMission != NULL)
				{
					return reqMission->completed;
				}
			}
		}
	}
	
	return 1;
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
