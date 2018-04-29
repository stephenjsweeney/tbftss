/*
Copyright (C) 2015-2018 Parallel Realities

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

#include "starfield.h"

Star stars[MAX_STARS];

void initStars(void)
{
	int i;
	
	memset(stars, 0, sizeof(Star) * MAX_STARS);
	
	for (i = 0 ; i < MAX_STARS ; i++)
	{
		stars[i].x = rand() % SCREEN_WIDTH;
		stars[i].y = rand() % SCREEN_HEIGHT;
		stars[i].speed = 5 + rand() % 35;
		
		stars[i].speed *= 0.1;
	}
}

void doStars(float dx, float dy)
{
	int i;
	
	for (i = 0 ; i < MAX_STARS ; i++)
	{
		stars[i].x -= (dx * stars[i].speed);
		stars[i].y -= (dy * stars[i].speed);
		
		stars[i].x = mod(stars[i].x, SCREEN_WIDTH - 1);
		stars[i].y = mod(stars[i].y, SCREEN_HEIGHT - 1);
	}
}

void drawStars(void)
{
	int i;
	int c;
	
	for (i = 0 ; i < MAX_STARS ; i++)
	{
		c = 64 * stars[i].speed;
		
		SDL_SetRenderDrawColor(app.renderer, c, c, c, 255);
		
		SDL_RenderDrawPoint(app.renderer, stars[i].x, stars[i].y);
		
		if (c >= 240)
		{
			SDL_RenderDrawPoint(app.renderer, stars[i].x + 1, stars[i].y + 1);
		}
	}
}
