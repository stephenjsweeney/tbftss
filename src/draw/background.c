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

#include "background.h"

static PointF backgroundPoint[4];
char **backgrounds;
char **planets;
int numBackgrounds;
int numPlanets;

void initBackground(void)
{
	char **filenames;
	int i;
	
	numBackgrounds = numPlanets = 0;
	
	filenames = getFileList(getFileLocation("gfx/backgrounds"), &numBackgrounds);
	backgrounds = malloc(sizeof(char*) * numBackgrounds);
	
	for (i = 0 ; i < numBackgrounds ; i++)
	{
		backgrounds[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
		sprintf(backgrounds[i], "gfx/backgrounds/%s", filenames[i]);
		
		free(filenames[i]);
	}
	
	free(filenames);
	
	filenames = getFileList("gfx/planets", &numPlanets);
	planets = malloc(sizeof(char*) * numPlanets);
	
	for (i = 0 ; i < numPlanets ; i++)
	{
		planets[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);
		sprintf(planets[i], "gfx/planets/%s", filenames[i]);
		
		free(filenames[i]);
	}
	
	free(filenames);
	
	backgroundPoint[0].x = -SCREEN_WIDTH / 2;
	backgroundPoint[0].y = -SCREEN_HEIGHT / 2;
	
	backgroundPoint[1].x = SCREEN_WIDTH / 2;
	backgroundPoint[1].y = -SCREEN_HEIGHT / 2;
	
	backgroundPoint[2].x = -SCREEN_WIDTH / 2;
	backgroundPoint[2].y = SCREEN_HEIGHT / 2;
	
	backgroundPoint[3].x = SCREEN_WIDTH / 2;
	backgroundPoint[3].y = SCREEN_HEIGHT / 2;
}

void scrollBackground(float x, float y)
{
	int i;
	
	for (i = 0 ; i < 4 ; i++)
	{
		backgroundPoint[i].x += x;
		backgroundPoint[i].y += y;
		
		if (backgroundPoint[i].x < 0)
		{
			backgroundPoint[i].x += (SCREEN_WIDTH * 2);
		}
		
		if (backgroundPoint[i].x >= SCREEN_WIDTH)
		{
			backgroundPoint[i].x -= (SCREEN_WIDTH * 2);
		}
		
		if (backgroundPoint[i].y < 0)
		{
			backgroundPoint[i].y += (SCREEN_HEIGHT * 2);
		}
		
		if (backgroundPoint[i].y >= SCREEN_HEIGHT)
		{
			backgroundPoint[i].y -= (SCREEN_HEIGHT * 2);
		}
	}
}

void drawBackground(SDL_Texture *texture)
{
	int i;
	
	for (i = 0 ; i < 4 ; i++)
	{
		blitScaled(texture, backgroundPoint[i].x, backgroundPoint[i].y, SCREEN_WIDTH, SCREEN_HEIGHT);
	}
}

char *getBackgroundTextureName(int i)
{
	return backgrounds[i % numBackgrounds];
}

char *getPlanetTextureName(int i)
{
	return planets[i % numPlanets];
}

void destroyBackground(void)
{
	free(backgrounds);
	free(planets);
}
