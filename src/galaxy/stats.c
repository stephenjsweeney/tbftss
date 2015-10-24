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

#include "stats.h"

static char *statDescription[] = {
	"Missions Started",
	"Missons Completed",
	"Shots Fired",
	"Shots Hit",
	"Missiles Fired",
	"Missiles Hit",
	"Enemies Killed",
	"Enemies Killed (Player)",
	"Allies Killed",
	"Times Killed",
	"Enemies Disabled",
	"STAT_TIME"
};

static int page;
static int maxPages;
static SDL_Texture *pagePrev;
static SDL_Texture *pageNext;

void initStatsDisplay(void)
{
	page = 0;
	maxPages = (STAT_MAX / MAX_STAT_ITEMS);
	
	pagePrev = getTexture("gfx/widgets/optionsLeft.png");
	pageNext = getTexture("gfx/widgets/optionsRight.png");
}

void handleStatsKB(void)
{
	if (app.keyboard[SDL_SCANCODE_LEFT])
	{
		page = MIN(MAX(page - 1, 0), maxPages);
	}
	
	if (app.keyboard[SDL_SCANCODE_RIGHT])
	{
		page = MIN(MAX(page + 1, 0), maxPages);
	}
}

void drawStats(void)
{
	int i, y, hours, minutes, seconds, startIndex;
	SDL_Rect r;
	char timePlayed[MAX_NAME_LENGTH];
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = 500;
	r.h = 600;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 70, 28, TA_CENTER, colors.white, "Stats");
	
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, "Page %d / %d", page + 1, maxPages + 1);
	
	if (page > 0)
	{
		blit(pagePrev, (SCREEN_WIDTH / 2) - 100, 120, 1);
	}
	
	if (page < maxPages)
	{
		blit(pageNext, (SCREEN_WIDTH / 2) + 100, 120, 1);
	}
	
	y = 170;
	
	startIndex = (page * MAX_STAT_ITEMS);
	
	for (i = startIndex ; i < startIndex + MAX_STAT_ITEMS ; i++)
	{
		if (i < STAT_TIME)
		{
			drawText(r.x + 20, y, 18, TA_LEFT, colors.white, statDescription[i]);
			drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats[i]);
			y += 40;
		}
	}
	
	seconds = game.stats[STAT_TIME] / FPS;
	minutes = (seconds / 60) % 60;
	hours = seconds / (60 * 60);
	
	seconds %= 60;
	
	sprintf(timePlayed, "%dh:%02dm:%02ds", hours, minutes, seconds);
	drawText(r.x + 20, 565, 18, TA_LEFT, colors.white, "Time Played");
	drawText(r.x + r.w - 20, 565, 18, TA_RIGHT, colors.white, timePlayed);
		
	drawWidgets("stats");	
}
