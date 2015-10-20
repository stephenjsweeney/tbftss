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
static void loadFighterGroups(cJSON *node);
static unsigned long hashcode(const char *str);

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
	battle.planet.x = rand() % SCREEN_WIDTH - rand() % SCREEN_WIDTH;
	battle.planet.y = rand() % SCREEN_HEIGHT - rand() % SCREEN_HEIGHT;
	
	loadObjectives(cJSON_GetObjectItem(root, "objectives"));
		
	loadPlayer(cJSON_GetObjectItem(root, "player"));
	
	loadFighters(cJSON_GetObjectItem(root, "fighters"));
	
	loadFighterGroups(cJSON_GetObjectItem(root, "fighterGroups"));
	
	STRNCPY(music, cJSON_GetObjectItem(root, "music")->valuestring, MAX_NAME_LENGTH);
	
	cJSON_Delete(root);
	free(text);
	
	srand(time(NULL));
	
	endSectionTransition();
	
	if (!battle.objectiveHead.next)
	{
		battle.status = MS_IN_PROGRESS;
	}
	
	playMusic(music);
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
			
			STRNCPY(o->description, cJSON_GetObjectItem(node, "description")->valuestring, MAX_DESCRIPTION_LENGTH);
			STRNCPY(o->targetName, cJSON_GetObjectItem(node, "targetName")->valuestring, MAX_NAME_LENGTH);
			o->targetValue = cJSON_GetObjectItem(node, "targetValue")->valueint;
			
			battle.objectiveTail->next = o;
			battle.objectiveTail = o;
			
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
	
	player = spawnFighter(type, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, side);
	player->defaultAction = NULL;
}

static void loadFighters(cJSON *node)
{
	Fighter *f;
	char *type;
	int side, x, y;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			type = cJSON_GetObjectItem(node, "type")->valuestring;
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			x = cJSON_GetObjectItem(node, "x")->valueint;
			y = cJSON_GetObjectItem(node, "y")->valueint;
			
			f = spawnFighter(type, x, y, side);
			
			STRNCPY(f->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		
			node = node->next;
		}
	}
}

static void loadFighterGroups(cJSON *node)
{
	Fighter *f;
	char *type, *name;
	int side, x, y;
	int number, i;
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			type = cJSON_GetObjectItem(node, "type")->valuestring;
			side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
			number = cJSON_GetObjectItem(node, "number")->valueint;
			x = cJSON_GetObjectItem(node, "x")->valueint;
			y = cJSON_GetObjectItem(node, "y")->valueint;
			name = cJSON_GetObjectItem(node, "name")->valuestring;
			
			for (i = 0 ; i < number ; i++)
			{
				f = spawnFighter(type, x, y, side);
				
				STRNCPY(f->name, name, MAX_NAME_LENGTH);
			}
		
			node = node->next;
		}
	}
}

Mission *getMission(StarSystem *starSystem, char *filename)
{
	Mission *mission;
	
	for (mission = starSystem->missionHead.next ; mission != NULL ; mission = mission->next)
	{
		if (strcmp(mission->filename, filename) == 0)
		{
			return mission;
		}
	}
	
	return NULL;
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
