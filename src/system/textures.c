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

#include "textures.h"

static Texture textures[NUM_TEXTURE_BUCKETS];

static void addTextureToCache(char *name, SDL_Texture *texture);
SDL_Texture *getTexture(char *name);

void initTextures(void)
{
	memset(&textures, 0, sizeof(Texture) * NUM_TEXTURE_BUCKETS);
}

static void addTextureToCache(char *name, SDL_Texture *texture)
{
	Texture *t, *new;
	int i;

	i = strlen(name) % NUM_TEXTURE_BUCKETS;

	t = &textures[i];

	/* horrible bit to look for the tail */
	while (t->next)
	{
		t = t->next;
	}

	new = malloc(sizeof(Texture));
	memset(new, 0, sizeof(Texture));

	STRNCPY(new->name, name, MAX_DESCRIPTION_LENGTH);
	new->texture = texture;

	t->next = new;
}

static SDL_Texture *loadTexture(char *filename)
{
	SDL_Texture *texture;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	texture = IMG_LoadTexture(app.renderer, filename);

	addTextureToCache(filename, texture);

	return texture;
}


SDL_Texture *getTexture(char *filename)
{
	Texture *t;
	int i;

	i = strlen(filename) % NUM_TEXTURE_BUCKETS;

	/* check if the texture is already loaded */
	for (t = textures[i].next ; t != NULL ; t = t->next)
	{
		if (strcmp(t->name, filename) == 0)
		{
			return t->texture;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "%s not in texture cache", filename);
	
	return loadTexture(filename);
}

void destroyTextures(void)
{
	Texture *t, *next;
	int i;

	for (i = 0 ; i < NUM_TEXTURE_BUCKETS ; i++)
	{
		t = textures[i].next;

		while (t)
		{
			next = t->next;
			SDL_DestroyTexture(t->texture);
			free(t);
			t = next;
		}

		textures[i].next = NULL;
	}
}
