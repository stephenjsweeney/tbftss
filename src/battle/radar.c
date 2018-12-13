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

#include "radar.h"

static AtlasImage *radarTexture;
static AtlasImage *radarWarningTexture;
static int radarRanges[] = {20, 40, 60};
static char *CAUTION_TEXT;

void initRadar(void)
{
	radarTexture = getAtlasImage("gfx/hud/radar.png");
	radarWarningTexture = getAtlasImage("gfx/hud/radarWarning.png");
	
	/* medium range by default */
	battle.radarRange = 1;
	
	CAUTION_TEXT = _("Caution: Leaving battle area - turn around.");
}

void drawRadar(void)
{
	SDL_Rect r;
	Entity *e;
	int dist, inRange, blink;
	
	blit(radarTexture, app.winWidth - 85, app.winHeight - 85, 1);
	
	drawText(app.winWidth - 160, app.winHeight - 30, 14, TA_RIGHT, colors.white, "%dx", battle.radarRange + 1);
	
	r.w = r.h = 3;
	
	blink = battle.stats[STAT_TIME] % 60 < 30;
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		dist = getDistance(e->x, e->y, player->x, player->y);
		
		if (e->active)
		{
			inRange = (!(e->flags & EF_SHORT_RADAR_RANGE)) ? (dist / radarRanges[battle.radarRange]) < 70 : dist < 500;
			
			if (inRange)
			{
				r.x = app.winWidth - 85;
				r.y = app.winHeight - 85;
				
				r.x -= (player->x - e->x) / radarRanges[battle.radarRange];
				r.y -= (player->y - e->y) / radarRanges[battle.radarRange];
				
				r.x--;
				r.y--;
				
				if (e->side == SIDE_NONE)
				{
					SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
				}
				else if (e->side == player->side)
				{
					SDL_SetRenderDrawColor(app.renderer, 0, 255, 0, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
				}
				
				if (e->type == ET_MINE || e->type == ET_SHADOW_MINE || e->type == ET_JUMPGATE || (e->owner && e->owner->type == ET_JUMPGATE))
				{
					SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
				}
				
				if (blink)
				{
					if (e == player->target || e == battle.missionTarget)
					{
						SDL_SetRenderDrawColor(app.renderer, 255, 255, 0, 255);
					}
					
					if (e->flags & EF_DISABLED)
					{
						SDL_SetRenderDrawColor(app.renderer, 0, 192, 255, 255);
					}
				}
				
				SDL_RenderFillRect(app.renderer, &r);
			}
		}
	}
}

void drawRadarRangeWarning(void)
{
	int x, y, leaving;
	
	x = (int)player->x / (app.winWidth / 2);
	y = (int)player->y / (app.winHeight / 2);
	leaving = 0;
	
	if (x <= 2 && player->dx < 0)
	{
		blitRotated(radarWarningTexture, app.winWidth - 85, app.winHeight - 85, 270);
		
		leaving = 1;
	}
	
	if (y <= 3 && player->dy < 0)
	{
		blitRotated(radarWarningTexture, app.winWidth - 85, app.winHeight - 85, 0);
		
		leaving = 1;
	}
	
	if (x >= BATTLE_AREA_CELLS - 2 && player->dx > 0)
	{
		blitRotated(radarWarningTexture, app.winWidth - 85, app.winHeight - 85, 90);
		
		leaving = 1;
	}
	
	if (y >= BATTLE_AREA_CELLS - 3 && player->dy > 0)
	{
		blitRotated(radarWarningTexture, app.winWidth - 85, app.winHeight - 85, 180);
		
		leaving = 1;
	}
	
	if (leaving && battle.stats[STAT_TIME] % FPS < 40)
	{
		drawText(app.winWidth / 2, app.winHeight - 30, 14, TA_CENTER, colors.white, CAUTION_TEXT);
	}
}
