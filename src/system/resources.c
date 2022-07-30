/*
Copyright (C) 2015-2019,2022 Parallel Realities

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

#include "../common.h"
#include "resources.h"
#include "../system/atlas.h"
#include "../system/io.h"

char **backgrounds;
char **planets;
char **musicFiles;
int numBackgrounds;
int numPlanets;
int numMusicFiles;

void initResources(void)
{
	char **filenames;
	int i;

	numBackgrounds = numPlanets = numMusicFiles = 0;

	filenames = getFileList("gfx/backgrounds", &numBackgrounds);
	backgrounds = malloc(sizeof(char*) * numBackgrounds);

	for (i = 0 ; i < numBackgrounds ; i++)
	{
		backgrounds[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
		sprintf(backgrounds[i], "gfx/backgrounds/%s", filenames[i]);

		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Adding AUTO %s", filenames[i]);

		free(filenames[i]);
	}

	free(filenames);

	filenames = getAtlasFileList("gfx/planets", &numPlanets);
	planets = malloc(sizeof(char*) * numPlanets);

	for (i = 0 ; i < numPlanets ; i++)
	{
		planets[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
		strcpy(planets[i], filenames[i]);

		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Adding AUTO %s", filenames[i]);

		free(filenames[i]);
	}

	free(filenames);

	filenames = getFileList("music/battle/", &numMusicFiles);

	musicFiles = malloc(sizeof(char*) * numMusicFiles);

	for (i = 0 ; i < numMusicFiles ; i++)
	{
		musicFiles[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
		sprintf(musicFiles[i], "music/battle/%s", filenames[i]);

		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Adding AUTO %s", filenames[i]);

		free(filenames[i]);
	}

	free(filenames);
}

char *getBackgroundTextureName(unsigned long i)
{
	return backgrounds[i % numBackgrounds];
}

char *getPlanetTextureName(unsigned long i)
{
	return planets[i % numPlanets];
}

char *getMusicFilename(unsigned long i)
{
	return musicFiles[i % numMusicFiles];
}

void destroyResources(void)
{
	int i;

	for (i = 0 ; i < numBackgrounds ; i++)
	{
		free(backgrounds[i]);
	}

	for (i = 0 ; i < numPlanets ; i++)
	{
		free(planets[i]);
	}

	for (i = 0 ; i < numMusicFiles ; i++)
	{
		free(musicFiles[i]);
	}
}
