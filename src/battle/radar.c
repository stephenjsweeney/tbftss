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

#include "radar.h"

#define RADAR_RANGE	20

static SDL_Texture *radarTexture;
static SDL_Texture *radarWarningTexture;

void initRadar(void)
{
	radarTexture = getTexture("gfx/hud/radar.png");
	radarWarningTexture = getTexture("gfx/hud/radarWarning.png");
}

void drawRadar(void)
{
	SDL_Rect r;
	Entity *f;
	
	blit(radarTexture, SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 1);
	
	r.w = r.h = 3;
	
	for (f = battle.entityHead.next ; f != NULL ; f = f->next)
	{
		if (f->active && getDistance(f->x, f->y, player->x, player->y) / RADAR_RANGE < 70)
		{
			r.x = SCREEN_WIDTH - 85;
			r.y = SCREEN_HEIGHT - 85;
			
			r.x -= (player->x - f->x) / RADAR_RANGE;
			r.y -= (player->y - f->y) / RADAR_RANGE;
			
			r.x--;
			r.y--;
			
			switch (f->side)
			{
				case SIDE_ALLIES:
					SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
					break;
					
				case SIDE_PIRATE:
				case SIDE_PANDORAN:
					SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
					break;
					
				case SIDE_NONE:
					SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
					break;
			}
			
			if (f == player->target)
			{
				SDL_SetRenderDrawColor(app.renderer, 255, 255, 0, 255);
			}
			
			if (f == battle.missionTarget)
			{
				SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
			}
			
			SDL_RenderFillRect(app.renderer, &r);
		}
	}
}

void drawRadarRangeWarning(void)
{
	int x, y, leaving;
	
	x = (int)player->x / GRID_CELL_WIDTH;
	y = (int)player->y / GRID_CELL_HEIGHT;
	leaving = 0;
	
	if (x <= 4)
	{
		blitRotated(radarWarningTexture, SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 270);
		
		leaving = 1;
	}
	
	if (y <= 4)
	{
		blitRotated(radarWarningTexture, SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 0);
		
		leaving = 1;
	}
	
	if (x >= GRID_SIZE - 4)
	{
		blitRotated(radarWarningTexture, SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 90);
		
		leaving = 1;
	}
	
	if (y >= GRID_SIZE - 4)
	{
		blitRotated(radarWarningTexture, SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 180);
		
		leaving = 1;
	}
	
	if (leaving && battle.stats[STAT_TIME] % FPS < 40)
	{
		drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50, 14, TA_CENTER, colors.red, "WARNING: Leaving battle area - turn around!");
	}
}
