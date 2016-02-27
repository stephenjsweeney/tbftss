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

#include "testMission.h"

static Mission mission;

static void loadChallenges(char *filename);

void loadTestMission(char *filename)
{
	memset(&mission, 0, sizeof(Mission));
	
	STRNCPY(mission.filename, filename, MAX_DESCRIPTION_LENGTH);
	
	game.currentMission = &mission;
	
	initBattle();
	
	loadChallenges(filename);
	
	loadMission(filename);
}

static void loadChallenges(char *filename)
{
	Challenge *challenge, *challengeTail;
	cJSON *root, *node;
	char *text;
	
	text = readFile(filename);
	
	root = cJSON_Parse(text);
	
	challengeTail = &mission.challengeHead;
	
	node = cJSON_GetObjectItem(root, "challenges");
	
	if (node)
	{
		node = node->child;
		
		while (node)
		{
			challenge = malloc(sizeof(Challenge));
			memset(challenge, 0, sizeof(Challenge));
			
			challenge->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			challenge->value = cJSON_GetObjectItem(node, "value")->valueint;
			
			challengeTail->next = challenge;
			challengeTail = challenge;
			
			node = node->next;
		}
	}
	
	cJSON_Delete(root);
	free(text);
}
