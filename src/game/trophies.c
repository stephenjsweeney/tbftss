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

#include "trophies.h"

static int recentlyAwarded;
static void loadTrophyData(char *filename);

void initTrophies(void)
{
	recentlyAwarded = 0;

	loadTrophyData("data/trophies/trophies.json");
}

void awardTrophy(char *id)
{
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (!t->awarded && strcmp(t->id, id) == 0)
		{
			t->awarded = 1;
			t->awardDate = time(NULL);
			t->notify = 1;

			recentlyAwarded++;
		}
	}
}

void drawTrophyAlerts(void)
{
	Trophy *t;

	if (recentlyAwarded)
	{
		for (t = game.trophyHead.next ; t != NULL ; t = t->next)
		{
			if (t->notify)
			{
				/* handle notification */
				return;
			}
		}
	}
}

void drawTrophies(void)
{
}

Trophy *getTrophy(char *id)
{
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (strcmp(t->id, id) == 0)
		{
			return t;
		}
	}

	return NULL;
}

static void loadTrophyData(char *filename)
{
	cJSON *root, *node;
	char *text;
	Trophy *t, *tail;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);
	root = cJSON_Parse(text);

	tail = &game.trophyHead;

	for (node = root->child ; node != NULL ; node = node->next)
	{
		t = malloc(sizeof(Trophy));
		memset(t, 0, sizeof(Trophy));

		STRNCPY(t->id, cJSON_GetObjectItem(node, "id")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(t->title, cJSON_GetObjectItem(node, "title")->valuestring, MAX_DESCRIPTION_LENGTH);
		STRNCPY(t->description, cJSON_GetObjectItem(node, "description")->valuestring, MAX_DESCRIPTION_LENGTH);
		t->value = lookup(cJSON_GetObjectItem(node, "value")->valuestring);
		t->hidden = getJSONValue(node, "hidden", 0);

		tail->next = t;
		tail = t;
	}

	cJSON_Delete(root);
	free(text);
}
