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
static void doChallenges(void);
static void startChallengeMission(void);
static void drawMenu(void);
static void resume(void);
static void stats(void);
static void trophies(void);
static void options(void);
static void ok(void);
static void returnFromOptions(void);
static void unlockChallenges(void);
static void quit(void);
static void updateChallengeMissionData(void);
static char *listRestrictions(void);
static void prevPage(void);
static void nextPage(void);

static SDL_Texture *background;
static SDL_Texture *planetTexture;
static SDL_Texture *challengeIcon;
static SDL_Texture *challengeIconHighlight;
static Widget *start;
static PointF planet;
static int show;
static int page;
static float maxPages;
static char timeLimit[MAX_DESCRIPTION_LENGTH];
static char restrictions[MAX_DESCRIPTION_LENGTH];
static int hasRestrictions;
static Widget *prev;
static Widget *next;

void initChallengeHome(void)
{
	Mission *m;
	
	startSectionTransition();

	stopMusic();

	updateAllMissions();

	unlockChallenges();

	awardChallengeTrophies();

	awardStatsTrophies();

	app.saveGame = 1;

	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

	background = getTexture(getBackgroundTextureName(rand()));
	planetTexture = getTexture(getPlanetTextureName(rand()));
	challengeIcon = getTexture("gfx/challenges/challengeIcon.png");
	challengeIconHighlight = getTexture("gfx/challenges/challengeIconHighlight.png");

	battle.camera.x =  battle.camera.y = 0;

	planet.x = rand() % SCREEN_WIDTH;
	planet.y = rand() % SCREEN_HEIGHT;
	
	maxPages = page = 0;
	
	for (m = game.challengeMissionHead.next ; m != NULL ; m = m->next)
	{
		maxPages++;
	}
	
	maxPages /= CHALLENGES_PER_PAGE;
	maxPages = ceil(maxPages);

	show = SHOW_CHALLENGES;

	initBackground();

	start = getWidget("start", "challenges");
	start->action = startChallengeMission;

	getWidget("resume", "challengesMenu")->action = resume;
	getWidget("stats", "challengesMenu")->action = stats;
	getWidget("trophies", "challengesMenu")->action = trophies;
	getWidget("options", "challengesMenu")->action = options;
	getWidget("quit", "challengesMenu")->action = quit;

	getWidget("ok", "stats")->action = ok;
	getWidget("ok", "trophies")->action = ok;
	
	prev = getWidget("prev", "challenges");
	prev->action = prevPage;
	prev->visible = 0;
	
	next = getWidget("next", "challenges");
	next->action = nextPage;
	next->visible = 1;

	/* select first challenge if none chosen */
	if (!game.currentMission)
	{
		game.currentMission = game.challengeMissionHead.next;
		updateChallengeMissionData();
	}

	endSectionTransition();

	playMusic("music/main/covert_operations.mp3", 1);
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

static void unlockChallenges(void)
{
	Mission *m;

	int i, prevCompleted;

	i = game.completedChallenges = game.totalChallenges = 0;

	prevCompleted = 1;

	for (m = game.challengeMissionHead.next ; m != NULL ; m = m->next)
	{
		m->available = (prevCompleted > 0 || dev.debug);

		game.completedChallenges += m->completedChallenges;
		game.totalChallenges += m->totalChallenges;
		prevCompleted = m->completedChallenges;

		i++;
	}
}

static void logic(void)
{
	handleKeyboard();

	scrollBackground(-0.25, 0);

	doStars(0.5, 0);

	planet.x -= 0.25;
	if (planet.x <= -200)
	{
		planet.x = SCREEN_WIDTH + 128 + (rand() % SCREEN_WIDTH);
		planet.y = (rand() % SCREEN_HEIGHT - 128);
	}

	switch (show)
	{
		case SHOW_CHALLENGES:
			doChallenges();
			break;

		case SHOW_MENU:
			break;

		case SHOW_STATS:
			break;

		case SHOW_OPTIONS:
			break;
	}

	doTrophyAlerts();

	doWidgets();
	
	doTrophies();
}

static void doChallenges(void)
{
	Mission *c;
	int i, startIndex, end;
	
	i = 0;
	startIndex = page * CHALLENGES_PER_PAGE;
	end = startIndex + CHALLENGES_PER_PAGE;
	
	for (c = game.challengeMissionHead.next ; c != NULL ; c = c->next)
	{
		if (i >= startIndex && i < end && app.mouse.button[SDL_BUTTON_LEFT] && collision(app.mouse.x, app.mouse.y, 3, 3, c->rect.x, c->rect.y, c->rect.w, c->rect.h))
		{
			if (c->available)
			{
				game.currentMission = c;

				updateChallengeMissionData();

				start->enabled = 1;

				playSound(SND_GUI_CLICK);
			}

			app.mouse.button[SDL_BUTTON_LEFT] = 0;
		}
		
		i++;
	}
}

static void updateChallengeMissionData(void)
{
	STRNCPY(timeLimit, timeToString(game.currentMission->challengeData.timeLimit, 0), MAX_DESCRIPTION_LENGTH);
	sprintf(restrictions, "%s", listRestrictions());
}

static void addRestriction(char *buffer, int restricted, char *description)
{
	if (restricted)
	{
		if (strlen(buffer) > 0)
		{
			strcat(buffer, ". ");
		}

		strcat(buffer, description);

		hasRestrictions = 1;
	}
}

static char *listRestrictions(void)
{
	static char textBuffer[MAX_DESCRIPTION_LENGTH];

	memset(textBuffer, '\0', MAX_DESCRIPTION_LENGTH);

	hasRestrictions = 0;

	addRestriction(textBuffer, game.currentMission->challengeData.noMissiles, _("No Missiles"));
	addRestriction(textBuffer, game.currentMission->challengeData.noECM, _("No ECM"));
	addRestriction(textBuffer, game.currentMission->challengeData.noBoost, _("No Boost"));
	addRestriction(textBuffer, game.currentMission->challengeData.noGuns, _("No Guns"));

	return strlen(textBuffer) > 0  ? textBuffer : "-";
}

static void draw(void)
{
	drawBackground(background);

	blit(planetTexture, planet.x, planet.y, 1);

	drawStars();
	
	drawText(SCREEN_WIDTH / 2, 40, 28, TA_CENTER, colors.white, _("Challenges"));
	drawText(SCREEN_WIDTH / 2, 83, 16, TA_CENTER, colors.lightGrey, _("Completed : %d / %d"), game.completedChallenges, game.totalChallenges);
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, _("Page : %d / %d"), page + 1, (int)maxPages);

	drawChallenges();

	switch (show)
	{
		case SHOW_CHALLENGES:
			drawWidgets("challenges");
			break;

		case SHOW_MENU:
			drawMenu();
			break;

		case SHOW_STATS:
			drawStats();
			break;
			
		case SHOW_TROPHIES:
			drawTrophies();
			break;

		case SHOW_OPTIONS:
			drawOptions();
			break;
	}

	drawTrophyAlert();
}

static void drawChallenges(void)
{
	Mission *m;
	Challenge *c;
	SDL_Rect r;
	int i, start, end;

	r.x = 135;
	r.y = 165;
	r.w = r.h = 96;

	start = page * CHALLENGES_PER_PAGE;
	end = start + CHALLENGES_PER_PAGE;
	
	i = 0;

	for (m = game.challengeMissionHead.next ; m != NULL ; m = m->next)
	{
		m->rect = r;

		if (i >= start && i < end)
		{
			if (game.currentMission == m)
			{
				blit(challengeIconHighlight, r.x, r.y, 0);
			}
			else
			{
				blit(challengeIcon, r.x, r.y, 0);
			}

			drawText(r.x + (r.w / 2), r.y + 28, 30, TA_CENTER, colors.white, "%d", i + 1);

			if (m->available)
			{
				drawText(r.x + (r.w / 2), r.y + r.w + 5, 18, TA_CENTER, (m->completedChallenges < m->totalChallenges) ? colors.white : colors.green, "%d / %d", m->completedChallenges, m->totalChallenges);
			}
			else
			{
				drawText(r.x + (r.w / 2), r.y + r.w + 5, 18, TA_CENTER, colors.lightGrey, _("[Locked]"));
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

	r.y = SCREEN_HEIGHT - 245;
	r.x = 100;
	r.w = SCREEN_WIDTH - 200;
	r.h = 150;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(app.renderer, &r);

	r.y = 240;

	if (game.currentMission)
	{
		drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - r.y, 24, TA_CENTER, colors.white, game.currentMission->description);

		r.y -= 50;
		c = game.currentMission->challengeData.challenges[0];
		drawText((SCREEN_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, colors.white, _("Craft: %s"), game.currentMission->craft);
		drawText((SCREEN_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "1. %s", getChallengeDescription(c));

		r.y -= 30;
		drawText((SCREEN_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, colors.white, _("Time Limit: %s"), timeLimit);

		c = game.currentMission->challengeData.challenges[1];
		if (c)
		{
			drawText((SCREEN_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "2. %s", getChallengeDescription(c));
		}

		r.y -= 30;
		drawText((SCREEN_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, hasRestrictions ? colors.red : colors.white, _("Restrictions: %s"), restrictions);

		c = game.currentMission->challengeData.challenges[2];
		if (c)
		{
			drawText((SCREEN_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "3. %s", getChallengeDescription(c));
		}
	}
}

static void drawMenu(void)
{
	SDL_Rect r;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

	r.w = 400;
	r.h = 500;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);

	drawWidgets("challengesMenu");
}

static void resume(void)
{
	show = SHOW_CHALLENGES;
}

static void options(void)
{
	show = SHOW_OPTIONS;

	initOptions(returnFromOptions);
}

static void stats(void)
{
	selectWidget("ok", "stats");

	show = SHOW_STATS;

	initStatsDisplay();
}

static void trophies(void)
{
	selectWidget("ok", "trophies");

	show = SHOW_TROPHIES;

	initTrophiesDisplay();
}

static void ok(void)
{
	selectWidget("resume", "challengesMenu");

	show = SHOW_MENU;
}

static void returnFromOptions(void)
{
	show = SHOW_MENU;

	selectWidget("resume", "challengesMenu");
}

static void quit(void)
{
	initTitle();
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE] && !app.awaitingWidgetInput)
	{
		switch (show)
		{
			case SHOW_CHALLENGES:
				selectWidget("resume", "challengesMenu");
				show = SHOW_MENU;
				playSound(SND_GUI_CLOSE);
				break;

			case SHOW_MENU:
				show = SHOW_CHALLENGES;
				break;

			case SHOW_OPTIONS:
			case SHOW_STATS:
			case SHOW_TROPHIES:
				show = SHOW_MENU;
				selectWidget("resume", "challengesMenu");
				break;
		}

		playSound(SND_GUI_CLOSE);

		clearInput();
	}
}

static void startChallengeMission(void)
{
	initBattle();

	loadMission(game.currentMission->filename);
}
