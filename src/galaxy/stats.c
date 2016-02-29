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
static void calculatePercentComplete(void);

static char *statDescription[STAT_MAX];

static int page;
static float maxPages;
static Widget *prev;
static Widget *next;

void initStats(void)
{
	statDescription[STAT_PERCENT_COMPLETE] = _("Percent Complete");
	statDescription[STAT_MISSIONS_STARTED] = _("Missions Started");
	statDescription[STAT_MISSIONS_COMPLETED] = _("Missons Completed");
	statDescription[STAT_CHALLENGES_STARTED] = _("Challenges Started");
	statDescription[STAT_CHALLENGES_COMPLETED] = _("Challenges Completed");
	statDescription[STAT_SHOTS_FIRED] = _("Shots Fired");
	statDescription[STAT_SHOTS_HIT] = _("Shots Hit");
	statDescription[STAT_ROCKETS_FIRED] = _("Rockets Fired");
	statDescription[STAT_ROCKETS_HIT] = _("Rockets Hit");
	statDescription[STAT_MISSILES_FIRED] = _("Missiles Fired");
	statDescription[STAT_MISSILES_HIT] = _("Missiles Hit");
	statDescription[STAT_ENEMIES_KILLED] = _("Enemies Killed");
	statDescription[STAT_ENEMIES_KILLED_PLAYER] = _("Enemies Killed (Player)");
	statDescription[STAT_ALLIES_KILLED] = _("Allies Killed");
	statDescription[STAT_PLAYER_KILLED] = _("Times Killed");
	statDescription[STAT_ENEMIES_DISABLED] = _("Enemies Disabled");
	statDescription[STAT_ENEMIES_ESCAPED] = _("Enemies Escaped");
	statDescription[STAT_ECM] = _("ECM Used");
	statDescription[STAT_BOOST] = _("Boost Used");
	statDescription[STAT_MISSILES_EVADED] = _("Missiles Evaded");
	statDescription[STAT_MISSILES_STRUCK] = _("Missiles Struck Player");
	statDescription[STAT_CIVILIANS_RESCUED] = _("Civilians Rescued");
	statDescription[STAT_CIVILIANS_KILLED] = _("Civilians Killed");
	statDescription[STAT_TUG] = _("Times used Tug");
	statDescription[STAT_SHUTTLE] = _("Times used Shuttle");
	statDescription[STAT_NUM_TOWED] = _("Craft Towed");
	statDescription[STAT_ITEMS_COLLECTED] = _("Items Collected");
	statDescription[STAT_EPIC_KILL_STREAK] = _("Longest Epic Kill Streak");
	statDescription[STAT_CAPITAL_SHIPS_DESTROYED] = _("Capital Ships Destroyed");
	statDescription[STAT_CAPITAL_SHIPS_LOST] = _("Capital Ships Lost");
	statDescription[STAT_TIME] = _("Time Played");
}

void initStatsDisplay(void)
{
	page = 0;
	
	maxPages = STAT_TIME;
	maxPages /= STATS_PER_PAGE;
	maxPages = ceil(maxPages);
	
	prev = getWidget("prev", "stats");
	prev->action = prevPage;
	prev->visible = 0;
	
	next = getWidget("next", "stats");
	next->action = nextPage;
	
	calculatePercentComplete();
}

static void calculatePercentComplete(void)
{
	StarSystem *starSystem;
	Mission *mission;
	int completed, total;
	
	completed = total = 0;
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		if (strcmp(starSystem->name, "Sol"))
		{
			completed += starSystem->completedMissions;
			total += starSystem->totalMissions;
		}
	}
	
	for (mission = game.challengeMissionHead.next ; mission != NULL ; mission = mission->next)
	{
		completed += mission->completedChallenges;
		total += mission->totalChallenges;
	}
	
	game.stats[STAT_PERCENT_COMPLETE] = getPercent(completed, total);
}

void drawStats(void)
{
	int i, y, startIndex;
	SDL_Rect r;
	
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
	
	drawText(SCREEN_WIDTH / 2, 70, 28, TA_CENTER, colors.white, _("Stats"));
	
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, _("Page %d / %d"), page + 1, (int)maxPages);
	
	y = 170;
	
	startIndex = (page * STATS_PER_PAGE);
	
	for (i = startIndex ; i < startIndex + STATS_PER_PAGE ; i++)
	{
		if (i < STAT_TIME)
		{
			drawText(r.x + 20, y, 18, TA_LEFT, colors.white, statDescription[i]);
			
			if (i == STAT_PERCENT_COMPLETE)
			{
				drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d%%", game.stats[i]);
			}
			else
			{
				drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats[i]);
			}
			
			y += 40;
		}
	}
	
	drawText(r.x + 20, 565, 18, TA_LEFT, colors.white, statDescription[STAT_TIME]);
	drawText(r.x + r.w - 20, 565, 18, TA_RIGHT, colors.white, timeToString(game.stats[STAT_TIME], 1));
		
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
