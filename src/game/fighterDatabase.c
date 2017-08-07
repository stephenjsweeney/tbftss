/*
Copyright (C) 2015-2017 Parallel Realities

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

#include "fighterDatabase.h"

static int page;
static int maxPages;
static Widget *prev;
static Widget *next;
static char *DB_TEXT;
static char *PAGE_TEXT;
static void prevFighter(void);
static void nextFighter(void);

void initFighterDatabase(void)
{
	DB_TEXT = _("Fighter Database");
	PAGE_TEXT = _("Page %d / %d");
}

void initFighterDatabaseDisplay(void)
{
	page = 0;
	maxPages = 1;
	
	prev = getWidget("prev", "fighterDB");
	prev->action = prevFighter;
	prev->visible = 0;
	
	next = getWidget("next", "fighterDB");
	next->action = nextFighter;
	next->visible = 1;
}

void drawFighterDatabase(void)
{
	SDL_Rect r;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = 800;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 50, 28, TA_CENTER, colors.white, DB_TEXT);
	
	drawText(SCREEN_WIDTH / 2, 90, 16, TA_CENTER, colors.lightGrey, PAGE_TEXT, page + 1, (int)maxPages);
	
	drawWidgets("fighterDB");
}

static void prevFighter(void)
{
}

static void nextFighter(void)
{
}
