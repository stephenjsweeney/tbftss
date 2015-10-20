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

void drawRadar(void)
{
	SDL_Rect r;
	Fighter *f;
	
	drawFilledCircle(SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 75, 0, 128, 0, 32);
	
	drawCircle(SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 25, 0, 255, 0, 64);
	drawCircle(SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 50, 0, 255, 0, 64);
	
	r.w = r.h = 3;
	
	for (f = battle.fighterHead.next ; f != NULL ; f = f->next)
	{
		if (getDistance(f->x, f->y, player->x, player->y) / 15 < 70)
		{
			r.x = SCREEN_WIDTH - 85;
			r.y = SCREEN_HEIGHT - 85;
			
			r.x -= (player->x - f->x) / 15;
			r.y -= (player->y - f->y) / 15;
			
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
			
			if (player->target == f)
			{
				SDL_SetRenderDrawColor(app.renderer, 255, 255, 0, 255);
			}
			
			SDL_RenderFillRect(app.renderer, &r);
		}
	}
	
	drawCircle(SCREEN_WIDTH - 85, SCREEN_HEIGHT - 85, 75, 0, 255, 0, 128);
}
