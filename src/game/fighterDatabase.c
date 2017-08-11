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

static void prevFighter(void);
static void nextFighter(void);
static int countFighterGuns(Entity *fighter, int type);

static int page;
static int maxPages;
static Widget *prev;
static Widget *next;
static char *DB_TEXT;
static char *PAGE_TEXT;
static const char *gunName[BT_MAX];
static Entity **dbFighters;
static float rotation;

void initFighterDatabase(void)
{
	DB_TEXT = _("Fighter Database");
	PAGE_TEXT = _("Page %d / %d");
	
	dbFighters = getDBFighters(&maxPages);
	
	gunName[BT_NONE] = "";
	gunName[BT_PARTICLE] = _("Particle Cannon");
	gunName[BT_PLASMA] = _("Plasma Cannon");
	gunName[BT_LASER] = _("Laser Cannon");
	gunName[BT_MAG] = _("Mag Cannon");
	gunName[BT_ROCKET] = _("Rockets");
	gunName[BT_MISSILE] = _("Missiles");
	
	rotation = 0;
}

void destroyFighterDatabase(void)
{
	free(dbFighters);
}

void initFighterDatabaseDisplay(void)
{
	page = 0;
	
	prev = getWidget("prev", "fighterDB");
	prev->action = prevFighter;
	
	next = getWidget("next", "fighterDB");
	next->action = nextFighter;
}

void doFighterDatabase(void)
{
	rotation++;
}

void drawFighterDatabase(void)
{
	SDL_Rect r;
	Entity *fighter;
	int i, y, numCannons;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = 700;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 50, 28, TA_CENTER, colors.white, DB_TEXT);
	
	drawText(SCREEN_WIDTH / 2, 90, 16, TA_CENTER, colors.lightGrey, PAGE_TEXT, page + 1, (int)maxPages);
	
	fighter = dbFighters[page];
	
	drawText(SCREEN_WIDTH / 2, 130, 28, TA_CENTER, colors.white, fighter->name);
	
	blitRotated(fighter->texture, r.x + (r.w / 2), 250, rotation);
	
	drawText(r.x + 25, 200, 22, TA_LEFT, colors.white, "Affiliation: %s", fighter->affiliation);
	drawText(r.x + 25, 240, 22, TA_LEFT, colors.white, "Armour: %d", fighter->health);
	drawText(r.x + 25, 280, 22, TA_LEFT, colors.white, "Shield: %d", fighter->shield);
	drawText(r.x + 25, 320, 22, TA_LEFT, colors.white, "Speed: %.0f", ((fighter->speed * fighter->speed) * FPS));
	
	y = 200;
	
	for (i = 1 ; i < BT_MAX ; i++)
	{
		numCannons = countFighterGuns(fighter, i);
		if (numCannons > 0)
		{
			drawText(r.x + r.w - 25, y, 22, TA_RIGHT, colors.white, "%s x %d", gunName[i], numCannons);
			
			y += 40;
		}
	}
	
	if (fighter->missiles > 0)
	{
		drawText(r.x + r.w - 25, y, 22, TA_RIGHT, colors.white, "Missiles x %d", fighter->missiles);
	}
	
	y = MAX(y, 320) + 75;
	
	limitTextWidth(r.w - 100);
	drawText(r.x + 25, y, 18, TA_LEFT, colors.white, fighter->description);
	limitTextWidth(0);
	
	drawWidgets("fighterDB");
}

static int countFighterGuns(Entity *fighter, int type)
{
	int i, num;
	
	num = 0;
	
	for (i = 0 ; i < MAX_FIGHTER_GUNS ; i++)
	{
		if (fighter->guns[i].type == type)
		{
			num++;
		}
	}
	
	return num;
}

static void prevFighter(void)
{
	page = mod(page - 1, maxPages);
}

static void nextFighter(void)
{
	page = mod(page + 1, maxPages);
}
