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

#include "challengeHome.h"

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void drawChallenges(void);
static void updateChallengeMissions(void);
static void doChallenges(void);
static void startChallengeMission(void);

static SDL_Texture *background;
static int startIndex;
static Widget *start;
static int completedChallenges;
static int totalChallenges;

void initChallengeHome(void)
{
	startSectionTransition();
	
	stopMusic();
	
	updateChallengeMissions();
	
	saveGame();
	
	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	background = getTexture("gfx/backgrounds/background04.jpg");
	
	battle.camera.x =  battle.camera.y = 0;
	
	game.currentMission = NULL;
	
	startIndex = 0;
	
	initBackground();
	
	start = getWidget("start", "challenges");
	start->enabled = 0;
	start->action = startChallengeMission;
	
	setMouse(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	
	endSectionTransition();
}

static void updateChallengeMissions(void)
{
	Mission *m;
	Challenge *c;
	int i;
	
	i = completedChallenges = totalChallenges = 0;
	
	for (m = game.challengeMissionHead.next ; m != NULL ; m = m->next)
	{
		m->available = i <= completedChallenges;
		
		m->totalChallenges = m->completedChallenges = 0;
		
		for (c = m->challengeHead.next ; c != NULL ; c = c->next)
		{
			m->totalChallenges++;
			
			if (c->passed)
			{
				m->completedChallenges++;
			}
		}
		
		completedChallenges += m->completedChallenges;
		totalChallenges += m->totalChallenges;
		
		i++;
	}
}

static void logic(void)
{
	handleKeyboard();
	
	scrollBackground(-0.25, 0);
	
	doStars(0.5, 0);
	
	doChallenges();
	
	doWidgets();
}

static void doChallenges(void)
{
	Mission *c;
	
	for (c = game.challengeMissionHead.next ; c != NULL ; c = c->next)
	{
		if (app.mouse.button[SDL_BUTTON_LEFT] && collision(app.mouse.x, app.mouse.y, 3, 3, c->rect.x, c->rect.y, c->rect.w, c->rect.h))
		{
			if (c->available)
			{
				game.currentMission = c;
			}
			else
			{
				
				
				game.currentMission = NULL;
			}
			
			start->enabled = c->available;
			
			app.mouse.button[SDL_BUTTON_LEFT] = 0;
		}
	}
}

static void draw(void)
{
	drawBackground(background);
	
	drawStars();
	
	drawText(SCREEN_WIDTH / 2, 50, 30, TA_CENTER, colors.white, "Challenges");
	
	drawText(SCREEN_WIDTH / 2, 100, 24, TA_CENTER, colors.white, "%d / %d", completedChallenges, totalChallenges);
	
	drawChallenges();
	
	drawWidgets("challenges");
}

static void drawChallenges(void)
{
	Mission *c;
	SDL_Rect r;
	int i, endIndex;
	
	r.x = 135;
	r.y = 185;
	r.w = r.h = 96;
	
	endIndex = startIndex + MAX_ITEMS;
	i = 0;
	
	for (c = game.challengeMissionHead.next ; c != NULL ; c = c->next)
	{
		c->rect = r;
		
		if (i >= startIndex && i <= endIndex)
		{
			SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
			SDL_RenderFillRect(app.renderer, &r);
			
			if (game.currentMission == c)
			{
				SDL_SetRenderDrawColor(app.renderer, 64, 128, 200, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(app.renderer, &r);
				SDL_SetRenderDrawColor(app.renderer, 128, 192, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawRect(app.renderer, &r);
			}
			else
			{
				SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawRect(app.renderer, &r);
			}
			
			drawText(r.x + (r.w / 2), r.y + 28, 30, TA_CENTER, colors.white, "%d", i + 1);
			
			if (c->available)
			{
				drawText(r.x + (r.w / 2), r.y + r.w + 5, 18, TA_CENTER, (c->completedChallenges < c->totalChallenges) ? colors.white : colors.green, "%d / %d", c->completedChallenges, c->totalChallenges);
			}
			else
			{
				drawText(r.x + (r.w / 2), r.y + r.w + 5, 18, TA_CENTER, colors.lightGrey, "[Locked]");
			}
			
			r.x += 150;
			
			if (r.x > SCREEN_WIDTH - 200)
			{
				r.y += 165;
				r.x = 135;
			}
		}
		
		i++;
	}
	
	if (game.currentMission)
	{
		drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 130, 24, TA_CENTER, colors.white, game.currentMission->description);
	}
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		initTitle();
	}
}

static void startChallengeMission(void)
{
	initBattle();
	
	loadMission(game.currentMission->filename);
}
