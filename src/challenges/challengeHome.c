/*
Copyright (C) 2015-2019,2022 Parallel Realities

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

#include "../common.h"

#include "../battle/battle.h"
#include "../battle/starfield.h"
#include "../challenges/challenges.h"
#include "../galaxy/mission.h"
#include "../game/fighterDatabase.h"
#include "../game/options.h"
#include "../game/stats.h"
#include "../game/title.h"
#include "../game/trophies.h"
#include "../system/atlas.h"
#include "../system/draw.h"
#include "../system/input.h"
#include "../system/resources.h"
#include "../system/sound.h"
#include "../system/text.h"
#include "../system/textures.h"
#include "../system/transition.h"
#include "../system/util.h"
#include "../system/widgets.h"
#include "challengeHome.h"

#define CHALLENGES_PER_PAGE 14
#define SHOW_CHALLENGES     0
#define SHOW_FIGHTER_DB     5
#define SHOW_MENU           1
#define SHOW_OPTIONS        2
#define SHOW_STATS          3
#define SHOW_TROPHIES       4

extern App    app;
extern Battle battle;
extern Colors colors;
extern Dev    dev;
extern Game   game;

static void  logic(void);
static void  draw(void);
static void  handleKeyboard(void);
static void  handleController(void);
static void  drawChallenges(void);
static void  doChallengeList(void);
static void  startChallengeMission(void);
static void  drawMenu(void);
static void  resume(void);
static void  stats(void);
static void  trophies(void);
static void  options(void);
static void  ok(void);
static void  returnFromOptions(void);
static void  unlockChallenges(void);
static void  quit(void);
static void  updateChallengeMissionData(void);
static char *listRestrictions(void);
static void  prevPage(void);
static void  nextPage(void);
static void  fighterDatabase(void);

static SDL_Texture *background;
static AtlasImage  *planetTexture;
static AtlasImage  *challengeIcon;
static AtlasImage  *challengeIconHighlight;
static Widget      *start;
static PointF       planet;
static int          show;
static int          page;
static float        maxPages;
static char         timeLimit[MAX_DESCRIPTION_LENGTH];
static char         restrictions[MAX_DESCRIPTION_LENGTH];
static int          hasRestrictions;
static Widget      *prev;
static Widget      *next;
static char        *CHALLENGES_TEXT;
static char        *COMPLETED_TEXT;
static char        *PAGE_TEXT;
static char        *LOCKED_TEXT;
static char        *CRAFT_TEXT;
static char        *TIME_TEXT;
static char        *RESTRICTIONS_TEXT;

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

	CHALLENGES_TEXT = _("Challenges");
	COMPLETED_TEXT = _("Completed : %d / %d");
	PAGE_TEXT = _("Page : %d / %d");
	LOCKED_TEXT = _("[Locked]");
	CRAFT_TEXT = _("Craft: %s");
	TIME_TEXT = _("Time Limit: %s");
	RESTRICTIONS_TEXT = _("Restrictions: %s");

	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

	background = getTexture(getBackgroundTextureName(rand()));
	planetTexture = getAtlasImage(getPlanetTextureName(rand()));
	challengeIcon = getAtlasImage("gfx/challenges/challengeIcon.png");
	challengeIconHighlight = getAtlasImage("gfx/challenges/challengeIconHighlight.png");

	battle.camera.x = battle.camera.y = 0;

	planet.x = rand() % app.winWidth;
	planet.y = rand() % app.winHeight;

	maxPages = page = 0;

	for (m = game.challengeMissionHead.next; m != NULL; m = m->next)
	{
		maxPages++;
	}

	maxPages /= CHALLENGES_PER_PAGE;
	maxPages = ceil(maxPages);

	show = SHOW_CHALLENGES;

	start = getWidget("start", "challenges");
	start->action = startChallengeMission;

	getWidget("resume", "challengesMenu")->action = resume;
	getWidget("stats", "challengesMenu")->action = stats;
	getWidget("trophies", "challengesMenu")->action = trophies;
	getWidget("fighterDB", "challengesMenu")->action = fighterDatabase;
	getWidget("options", "challengesMenu")->action = options;
	getWidget("quit", "challengesMenu")->action = quit;

	getWidget("ok", "stats")->action = ok;
	getWidget("ok", "trophies")->action = ok;
	getWidget("ok", "fighterDB")->action = ok;

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

	SDL_SetWindowGrab(app.window, 0);

	autoSizeWidgetButtons("challenges", 1);

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

	for (m = game.challengeMissionHead.next; m != NULL; m = m->next)
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
	handleController();

	handleKeyboard();

	scrollBackground(-0.25, 0);

	doStars(0.5, 0);

	planet.x -= 0.25;
	if (planet.x <= -200)
	{
		planet.x = app.winWidth + 128 + (rand() % app.winWidth);
		planet.y = (rand() % app.winHeight - 128);
	}

	switch (show)
	{
		case SHOW_CHALLENGES:
			doChallengeList();
			break;

		case SHOW_MENU:
			break;

		case SHOW_STATS:
			break;

		case SHOW_OPTIONS:
			break;
	}

	doWidgets();

	if (show == SHOW_FIGHTER_DB)
	{
		doFighterDatabase();
	}

	app.doTrophyAlerts = 1;
}

static void doChallengeList(void)
{
	Mission *c;
	int      i, startIndex, end;

	i = 0;
	startIndex = page * CHALLENGES_PER_PAGE;
	end = startIndex + CHALLENGES_PER_PAGE;

	for (c = game.challengeMissionHead.next; c != NULL; c = c->next)
	{
		if (i >= startIndex && i < end && app.mouse.button[SDL_BUTTON_LEFT] && collision(app.uiMouse.x, app.uiMouse.y, 3, 3, c->rect.x, c->rect.y, c->rect.w, c->rect.h))
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

	return strlen(textBuffer) > 0 ? textBuffer : "-";
}

static void draw(void)
{
	drawBackground(background);

	blit(planetTexture, planet.x, planet.y, 1);

	drawStars();

	if (show == SHOW_MENU)
	{
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		SDL_RenderFillRect(app.renderer, NULL);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	}

	SDL_SetRenderTarget(app.renderer, app.uiBuffer);

	drawText(UI_WIDTH / 2, 40, 28, TA_CENTER, colors.white, CHALLENGES_TEXT);
	drawText(UI_WIDTH / 2, 83, 16, TA_CENTER, colors.lightGrey, COMPLETED_TEXT, game.completedChallenges, game.totalChallenges);
	drawText(UI_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, PAGE_TEXT, page + 1, (int)maxPages);

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

		case SHOW_FIGHTER_DB:
			drawFighterDatabase();
			break;
	}

	SDL_SetRenderTarget(app.renderer, app.backBuffer);
}

static void drawChallenges(void)
{
	Mission   *m;
	Challenge *c;
	SDL_Rect   r;
	int        i, start, end;

	r.x = 135;
	r.y = 165;
	r.w = r.h = 96;

	start = page * CHALLENGES_PER_PAGE;
	end = start + CHALLENGES_PER_PAGE;

	i = 0;

	for (m = game.challengeMissionHead.next; m != NULL; m = m->next)
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
				drawText(r.x + (r.w / 2), r.y + r.w + 5, 18, TA_CENTER, colors.lightGrey, LOCKED_TEXT);
			}

			r.x += 150;

			if (r.x > UI_WIDTH - 200)
			{
				r.y += 165;
				r.x = 135;
			}
		}

		i++;
	}

	r.y = UI_HEIGHT - 245;
	r.x = 100;
	r.w = UI_WIDTH - 200;
	r.h = 150;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, 255);
	SDL_RenderDrawRect(app.renderer, &r);

	r.y = 240;

	if (game.currentMission)
	{
		drawText(UI_WIDTH / 2, SCREEN_HEIGHT - r.y, 24, TA_CENTER, colors.white, game.currentMission->description);

		r.y -= 50;
		c = game.currentMission->challengeData.challenges[0];
		drawText((UI_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, colors.white, CRAFT_TEXT, game.currentMission->craft);
		drawText((UI_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "1. %s", getChallengeDescription(c));

		r.y -= 30;
		drawText((UI_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, colors.white, TIME_TEXT, timeLimit);

		c = game.currentMission->challengeData.challenges[1];
		if (c)
		{
			drawText((UI_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "2. %s", getChallengeDescription(c));
		}

		r.y -= 30;
		drawText((UI_WIDTH / 2) - 25, SCREEN_HEIGHT - r.y, 18, TA_RIGHT, hasRestrictions ? colors.red : colors.white, RESTRICTIONS_TEXT, restrictions);

		c = game.currentMission->challengeData.challenges[2];
		if (c)
		{
			drawText((UI_WIDTH / 2) + 25, SCREEN_HEIGHT - r.y, 18, TA_LEFT, (c->passed) ? colors.green : colors.white, "3. %s", getChallengeDescription(c));
		}
	}
}

static void drawMenu(void)
{
	SDL_Rect r;

	r.w = 400;
	r.h = 500;
	r.x = (UI_WIDTH / 2) - r.w / 2;
	r.y = (UI_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
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

static void fighterDatabase(void)
{
	show = SHOW_FIGHTER_DB;

	initFighterDatabaseDisplay();
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
	if ((app.keyboard[SDL_SCANCODE_ESCAPE] || app.controllerButton[CONTROL_BOOST]) && !app.awaitingWidgetInput)
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
			case SHOW_FIGHTER_DB:
				show = SHOW_MENU;
				selectWidget("resume", "challengesMenu");
				break;
		}

		playSound(SND_GUI_CLOSE);

		clearInput();
		app.controllerButton[CONTROL_BOOST] = 0;
	}
}

static void handleController(void)
{
	if ((app.controllerAxis[0] != 0) || (app.controllerAxis[0] != 0) || (app.controllerAxis[0] != 0) || (app.controllerAxis[0] != 0))
	{
		if (app.controllerX == CONTROLLER_NOINPUT)
		{
			app.controllerX = app.mouse.x;
			app.controllerY = app.mouse.y;
		}
		app.controllerX += app.controllerAxis[0] / 4;
		if (app.controllerX < 0)
		{
			app.controllerX = 0;
		}
		else if (app.controllerX > app.winWidth)
		{
			app.controllerX = app.winWidth;
		}
		app.uiMouse.x = app.controllerX - app.uiOffset.x;
		app.controllerY += app.controllerAxis[1] / 4;
		if (app.controllerY < 0)
		{
			app.controllerY = 0;
		}
		else if (app.controllerY > app.winHeight)
		{
			app.controllerY = app.winHeight;
		}
		app.uiMouse.y = app.controllerY - app.uiOffset.y;
	}
}

static void startChallengeMission(void)
{
	initBattle();

	loadMission(game.currentMission->filename);
}
