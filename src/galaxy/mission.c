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
static void loadTriggers(cJSON *node);
static void loadPlayer(cJSON *node);
static void loadFighters(cJSON *node);
static void loadFighterGroups(cJSON *node);
static void loadEntities(cJSON *node);
static void loadEntityGroups(cJSON *node);
static void loadItems(cJSON *node);
static void loadItemGroups(cJSON *node);
static unsigned long hashcode(const char *str);
static char **toFighterTypeArray(char *types, int *numTypes);
static void loadEpicData(cJSON *node);

void loadMission(char *filename)
{
	cJSON *root;
	char *text, music[MAX_NAME_LENGTH];
	
	startSectionTransition();
	
	stopMusic();
	
	text = readFile(filename);
	
	srand(hashcode(filename));
	
	root = cJSON_Parse(text);
	
	battle.background = getTexture(cJSON_GetObjectItem(root, "background")->valuestring);
	battle.planetTexture = getTexture(cJSON_GetObjectItem(root, "planet")->valuestring);
	battle.planet.x = ((200 + rand() % 100) / 10) * GRID_CELL_WIDTH;
	battle.planet.y = ((200 + rand() % 100) / 10) * GRID_CELL_HEIGHT;
	
	loadObjectives(cJSON_GetObjectItem(root, "objectives"));
	
	loadTriggers(cJSON_GetObjectItem(root, "triggers"));
		
	loadPlayer(cJSON_GetObjectItem(root, "player"));
	
	loadFighters(cJSON_GetObjectItem(root, "fighters"));
	
	loadFighterGroups(cJSON_GetObjectItem(root, "fighterGroups"));
	
	loadEntities(cJSON_GetObjectItem(root, "entities"));
	
	loadEntityGroups(cJSON_GetObjectItem(root, "entityGroups"));
	
	loadItems(cJSON_GetObjectItem(root, "items"));
	
	loadItemGroups(cJSON_GetObjectItem(root, "itemGroups"));
	
	STRNCPY(music, cJSON_GetObjectItem(root, "music")->valuestring, MAX_NAME_LENGTH);
	
	if (cJSON_GetObjectItem(root, "epic"))
	{
		loadEpicData(cJSON_GetObjectItem(root, "epic"));
	}
	
	cJSON_Delete(root);
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
		
		retreatEnemies();
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

static void loadTriggers(cJSON *node)
{
	Trigger *t;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			t = malloc(sizeof(Trigger));
			memset(t, 0, sizeof(Trigger));
			battle.triggerTail->next = t;
			battle.triggerTail = t;
			
			t->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			STRNCPY(t->targetName, cJSON_GetObjectItem(node, "targetName")->valuestring, MAX_NAME_LENGTH);
			t->targetValue = cJSON_GetObjectItem(node, "targetValue")->valueint;
			t->action = lookup(cJSON_GetObjectItem(node, "action")->valuestring);
			
			if (cJSON_GetObjectItem(node, "actionValue"))
			{
				STRNCPY(t->actionValue, cJSON_GetObjectItem(node, "actionValue")->valuestring, MAX_NAME_LENGTH);
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
	Entity *f;
	char *type;
	int side;
	float x, y;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			type = cJSON_GetObjectItem(node, "type")->valuestring;
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			
			f = spawnFighter(type, x, y, side);
			
			STRNCPY(f->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			
			if (cJSON_GetObjectItem(node, "flags"))
			{
				f->flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring);
			}
			
			if (cJSON_GetObjectItem(node, "aiFlags"))
			{
				f->aiFlags = flagsToLong(cJSON_GetObjectItem(node, "aiFlags")->valuestring);
			}
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				f->active = cJSON_GetObjectItem(node, "active")->valueint;
			}
		
			node = node->next;
		}
	}
}

static void loadFighterGroups(cJSON *node)
{
	Entity *f;
	char **types, *name, *groupName, *type;
	int side, scatter, number, active;
	int i, numTypes;
	long flags;
	float x, y;
	
	scatter = 1;
	active = 1;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			groupName = NULL;
			flags = -1;
			
			types = toFighterTypeArray(cJSON_GetObjectItem(node, "types")->valuestring, &numTypes);
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			number = cJSON_GetObjectItem(node, "number")->valueint;
			name = cJSON_GetObjectItem(node, "name")->valuestring;
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
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
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring);
			}
			
			for (i = 0 ; i < number ; i++)
			{
				type = types[rand() % numTypes];
				
				f = spawnFighter(type, x, y, side);
				
				f->x += (rand() % scatter) - (rand() % scatter);
				f->y += (rand() % scatter) - (rand() % scatter);
				
				f->active = active;
				
				if (flags != -1)
				{
					f->flags = flags;
				}
				
				STRNCPY(f->name, name, MAX_NAME_LENGTH);
				
				if (groupName)
				{
					STRNCPY(f->groupName, groupName, MAX_NAME_LENGTH);
				}
			}
		
			node = node->next;
			
			free(types);
		}
	}
}

static void loadEntities(cJSON *node)
{
	Entity *e;
	int type;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			
			switch (type)
			{
				case ET_WAYPOINT:
					e = spawnWaypoint();
					break;
					
				case ET_EXTRACTION_POINT:
					e = spawnExtractionPoint();
					break;
			}
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				STRNCPY(e->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			}
			
			e->x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			e->y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			
			if (cJSON_GetObjectItem(node, "active"))
			{
				e->active = cJSON_GetObjectItem(node, "active")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "flags"))
			{
				e->flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring);
			}
			
			SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
		
			node = node->next;
		}
	}
}

static void loadEntityGroups(cJSON *node)
{
	Entity *e;
	char *name, *groupName;
	int i, type, scatter, number;
	float x, y;
	
	scatter = 1;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			number = cJSON_GetObjectItem(node, "number")->valueint;
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			name = NULL;
			groupName = NULL;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
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
				
				e->x += (rand() % scatter) - (rand() % scatter);
				e->y += (rand() % scatter) - (rand() % scatter);
				
				SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			}
		
			node = node->next;
		}
	}
}

static void loadItems(cJSON *node)
{
}

static void loadItemGroups(cJSON *node)
{
	Entity *e;
	char *name, *groupName, *type;
	int i, scatter, number;
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
			number = cJSON_GetObjectItem(node, "number")->valueint;
			x = cJSON_GetObjectItem(node, "x")->valuedouble * GRID_CELL_WIDTH;
			y = cJSON_GetObjectItem(node, "y")->valuedouble * GRID_CELL_HEIGHT;
			name = NULL;
			groupName = NULL;
			
			if (cJSON_GetObjectItem(node, "name"))
			{
				name = cJSON_GetObjectItem(node, "name")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "groupName"))
			{
				groupName = cJSON_GetObjectItem(node, "groupName")->valuestring;
			}
			
			if (cJSON_GetObjectItem(node, "scatter"))
			{
				scatter = cJSON_GetObjectItem(node, "scatter")->valueint;
			}
			
			if (cJSON_GetObjectItem(node, "flags"))
			{
				flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring);
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
					e->flags = flags;
				}
				
				e->x = x;
				e->y = y;
				
				e->x += (rand() % scatter) - (rand() % scatter);
				e->y += (rand() % scatter) - (rand() % scatter);
				
				SDL_QueryTexture(e->texture, NULL, NULL, &e->w, &e->h);
			}
		
			node = node->next;
		}
	}
}

static char **toFighterTypeArray(char *types, int *numTypes)
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
	#if !DEBUG
	Mission *reqMission;
	
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
	#endif
	
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
