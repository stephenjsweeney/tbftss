/*
Copyright (C) 2018 Parallel Realities

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

#include "atlas.h"

static void loadAtlasData(void);

static AtlasImage atlases[NUM_ATLAS_BUCKETS];
static SDL_Texture *atlasTexture;

void initAtlas(void)
{
	memset(&atlases, 0, sizeof(AtlasImage) * NUM_ATLAS_BUCKETS);
	
	atlasTexture = getTexture("gfx/atlas/atlas.png");

	loadAtlasData();
}

void setAtlasColor(int r, int g, int b, int a)
{
	SDL_SetTextureColorMod(atlasTexture, r, g, b);
	SDL_SetTextureAlphaMod(atlasTexture, a);
}

AtlasImage *getAtlasImage(char *filename)
{
	AtlasImage *a;
	unsigned long i;

	i = hashcode(filename) % NUM_ATLAS_BUCKETS;
	
	for (a = atlases[i].next ; a != NULL ; a = a->next)
	{
		if (strcmp(a->filename, filename) == 0)
		{
			return a;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, "No such atlas image '%s'", filename);
	exit(1);
	
	return NULL;
}

char **getAtlasFileList(char *dir, int *count)
{
	AtlasImage *a;
	int i, bucket;
	char **filenames;

	i = 0;
	filenames = NULL;

	for (bucket = 0 ; bucket < NUM_ATLAS_BUCKETS ; bucket++)
	{
		for (a = atlases[bucket].next ; a != NULL ; a = a->next)
		{
			if (strncmp(dir, a->filename, strlen(dir)) == 0)
			{
				i++;
			}
		}
	}
	
	if (i > 0)
	{
		filenames = malloc(sizeof(char*) * i);
		memset(filenames, 0, sizeof(char*) * i);

		i = 0;

		for (bucket = 0 ; bucket < NUM_ATLAS_BUCKETS ; bucket++)
		{
			for (a = atlases[bucket].next ; a != NULL ; a = a->next)
			{
				if (strncmp(dir, a->filename, strlen(dir)) == 0)
				{
					filenames[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);

					STRNCPY(filenames[i], a->filename, MAX_FILENAME_LENGTH);

					i++;
				}
			}
		}
	}

	*count = i;

	if (filenames)
	{
		qsort(filenames, i, sizeof(char*), stringComparator);
	}

	return filenames;
}

static void loadAtlasData(void)
{
	AtlasImage *atlasImage, *a;
	int x, y, w, h;
	cJSON *root, *node;
	char *text, *filename;
	unsigned long i;
	
	text = readFile("data/atlas/atlas.json");

	root = cJSON_Parse(text);
	
	for (node = root->child ; node != NULL ; node = node->next)
	{
		filename = cJSON_GetObjectItem(node, "filename")->valuestring;
		x = cJSON_GetObjectItem(node, "x")->valueint;
		y = cJSON_GetObjectItem(node, "y")->valueint;
		w = cJSON_GetObjectItem(node, "w")->valueint;
		h = cJSON_GetObjectItem(node, "h")->valueint;

		i = hashcode(filename) % NUM_ATLAS_BUCKETS;

		a = &atlases[i];

		/* horrible bit to look for the tail */
		while (a->next)
		{
			a = a->next;
		}
		
		atlasImage = malloc(sizeof(AtlasImage));
		memset(atlasImage, 0, sizeof(AtlasImage));
		a->next = atlasImage;
		
		STRNCPY(atlasImage->filename, filename, MAX_FILENAME_LENGTH);
		atlasImage->rect.x = x;
		atlasImage->rect.y = y;
		atlasImage->rect.w = w;
		atlasImage->rect.h = h;
		
		atlasImage->texture = atlasTexture;
	}
	
	cJSON_Delete(root);
	
	free(text);
}
