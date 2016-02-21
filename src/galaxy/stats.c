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

#include "stats.h"

static void prevPage(void);
static void nextPage(void);

static char *statDescription[] = {
	"Missions Started",
	"Missons Completed",
	"Shots Fired",
	"Shots Hit",
	"Rockets Fired",
	"Rockets Hit",
	"Missiles Fired",
	"Missiles Hit",
	"Enemies Killed",
	"Enemies Killed (Player)",
	"Allies Killed",
	"Times Killed",
	"Enemies Disabled",
	"Enemies Escaped",
	"ECM Used",
	"Boost Used",
	"Missiles Evaded",
	"Missiles Struck Player",
	"Civilians Rescued",
	"Civilians Killed",
	"Times used Tug",
	"Times used Shuttle",
	"Craft Towed",
	"Items Collected",
	"Longest Epic Kill Streak",
	"Capital Ships Destroyed",
	"Capital Ships Lost",
	"STAT_TIME"
};

static int page;
static int maxPages;
static Widget *prev;
static Widget *next;

void initStatsDisplay(void)
{
	page = 0;
	maxPages = ceil(STAT_TIME / MAX_STAT_ITEMS);
	
	prev = getWidget("prev", "stats");
	prev->action = prevPage;
	prev->visible = 0;
	
	next = getWidget("next", "stats");
	next->action = nextPage;
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
	
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, "Page %d / %d", page + 1, maxPages);
	
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

static void nextPage(void)
{
	page = MIN(page + 1, maxPages - 1);
	
	next->visible = page < maxPages - 1;
	prev->visible = 1;
}

static void prevPage(void)
{
	page = MAX(0, page - 1);
	
	next->visible = 1;
	prev->visible = page > 0;
}
