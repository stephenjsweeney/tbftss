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
#include "../battle/effects.h"
#include "../battle/starfield.h"
#include "../challenges/challengeHome.h"
#include "../galaxy/galacticMap.h"
#include "../galaxy/mission.h"
#include "../game/credits.h"
#include "../game/fighterDatabase.h"
#include "../game/options.h"
#include "../game/stats.h"
#include "../game/trophies.h"
#include "../system/atlas.h"
#include "../system/draw.h"
#include "../system/input.h"
#include "../system/sound.h"
#include "../system/text.h"
#include "../system/textures.h"
#include "../system/transition.h"
#include "../system/widgets.h"
#include "title.h"

#define NUM_FIGHTERS    12
#define SHOW_FIGHTER_DB 4
#define SHOW_OPTIONS    2
#define SHOW_STATS      1
#define SHOW_TITLE      0
#define SHOW_TROPHIES   3

extern App     app;
extern Battle  battle;
extern Colors  colors;
extern Entity *self;
extern Game    game;

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void handleController(void);
static void initFighters(void);
static void doFighters(void);
static void drawFighters(void);
static void campaign(void);
static void challenges(void);
static void trophies(void);
static void stats(void);
static void fighterDatabase(void);
static void ok(void);
static void options(void);
static void credits(void);
static void quit(void);
static void returnFromOptions(void);

static SDL_Texture *background;
static AtlasImage  *logo[2];
static AtlasImage  *pandoranWar;
static AtlasImage  *earthTexture;
static PointF       earth;
static Entity       fighters[NUM_FIGHTERS];
static const char  *fighterTextures[] = {"gfx/fighters/firefly.png", "gfx/fighters/hammerhead.png", "gfx/fighters/hyena.png", "gfx/fighters/lynx.png", "gfx/fighters/kingfisher.png", "gfx/fighters/leopard.png", "gfx/fighters/nymph.png", "gfx/fighters/ray.png", "gfx/fighters/rook.png", "gfx/fighters/taf.png"};
static int          show;

void initTitle(void)
{
	startSectionTransition();

	stopMusic();

	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

	battle.camera.x = battle.camera.y = 0;

	destroyBattle();

	logo[0] = getAtlasImage("gfx/title/logo01.png");
	logo[1] = getAtlasImage("gfx/title/logo02.png");

	pandoranWar = getAtlasImage("gfx/title/pandoran.png");

	background = getTexture("gfx/backgrounds/background02.jpg");

	earthTexture = getAtlasImage("gfx/planets/earth.png");

	earth.x = rand() % app.winWidth;
	earth.y = -(128 + (rand() % 128));

	initEffects();

	initFighters();

	updateAllMissions();

	getWidget("campaign", "title")->action = campaign;
	getWidget("challenges", "title")->action = challenges;
	getWidget("trophies", "title")->action = trophies;
	getWidget("stats", "title")->action = stats;
	getWidget("fighterDB", "title")->action = fighterDatabase;
	getWidget("options", "title")->action = options;
	getWidget("credits", "title")->action = credits;
	getWidget("quit", "title")->action = quit;

	getWidget("ok", "stats")->action = ok;
	getWidget("ok", "trophies")->action = ok;
	getWidget("ok", "fighterDB")->action = ok;

	autoSizeWidgetButtons("title", 1);

	show = SHOW_TITLE;

	endSectionTransition();

	playMusic("music/main/Rise of spirit.ogg", 0);
}

static void initFighters(void)
{
	int i, numTextures;

	numTextures = sizeof(fighterTextures) / sizeof(char *);

	memset(&fighters, 0, sizeof(Entity) * NUM_FIGHTERS);

	for (i = 0; i < NUM_FIGHTERS; i++)
	{
		fighters[i].x = rand() % (app.winWidth - 32);
		fighters[i].y = app.winHeight + (rand() % app.winHeight);
		fighters[i].texture = getAtlasImage(fighterTextures[rand() % numTextures]);
		fighters[i].dy = -(1 + rand() % 3);
	}
}

static void logic(void)
{
	handleController();

	handleKeyboard();

	scrollBackground(0, 0.25);

	doStars(0, -0.5);

	earth.y += 0.1;

	if (earth.y > app.winHeight + 128)
	{
		earth.x = rand() % app.winWidth;
		earth.y = -(128 + (rand() % 128));
	}

	doFighters();

	doEffects();

	app.doTrophyAlerts = 1;

	if (show == SHOW_FIGHTER_DB)
	{
		doFighterDatabase();
	}

	doWidgets();
}

static void doFighters(void)
{
	int i, numTextures;

	numTextures = sizeof(fighterTextures) / sizeof(char *);

	for (i = 0; i < NUM_FIGHTERS; i++)
	{
		self = &fighters[i];

		/* engine position hack, due to camera being fixed */
		self->y += 16;
		addEngineEffect();
		self->y -= 16;

		self->y += self->dy;

		if (self->y <= -64)
		{
			self->x = rand() % (app.winWidth - 32);
			self->y = app.winHeight + (rand() % app.winHeight);
			self->texture = getAtlasImage(fighterTextures[rand() % numTextures]);
			self->dy = -(1 + rand() % 3);
		}
	}
}

static void draw(void)
{
	drawBackground(background);

	drawStars();

	setAtlasColor(255, 255, 255, 255);

	blit(earthTexture, earth.x, earth.y, 1);

	drawFighters();

	drawEffects();

	setAtlasColor(255, 255, 255, 255);

	blit(logo[0], (app.winWidth / 2) - logo[0]->rect.w, 30, 0);
	blit(logo[1], (app.winWidth / 2), 30, 0);

	blit(pandoranWar, app.winWidth / 2, 110, 1);

	drawText(10, app.winHeight - 25, 14, TA_LEFT, colors.white, "Copyright Parallel Realities, 2015-2019,2022");
	drawText(app.winWidth - 10, app.winHeight - 25, 14, TA_RIGHT, colors.white, "Version %.2f.%d", VERSION, REVISION);

	switch (show)
	{
		case SHOW_TITLE:
			SDL_SetRenderTarget(app.renderer, app.uiBuffer);
			drawWidgets("title");
			SDL_SetRenderTarget(app.renderer, app.backBuffer);
			break;

		case SHOW_STATS:
			drawStats();
			break;

		case SHOW_OPTIONS:
			drawOptions();
			break;

		case SHOW_TROPHIES:
			drawTrophies();
			break;

		case SHOW_FIGHTER_DB:
			drawFighterDatabase();
			break;
	}
}

static void drawFighters(void)
{
	int i;

	setAtlasColor(255, 255, 255, 255);

	for (i = 0; i < NUM_FIGHTERS; i++)
	{
		blit(fighters[i].texture, fighters[i].x, fighters[i].y, 1);
	}
}

static void handleKeyboard(void)
{
	if ((app.keyboard[SDL_SCANCODE_ESCAPE] || app.controllerButton[CONTROL_BOOST]) && !app.awaitingWidgetInput)
	{
		returnFromOptions();
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

static void campaign(void)
{
	destroyBattle();

	initGalacticMap();
}

static void challenges(void)
{
	destroyBattle();

	game.currentMission = NULL;

	initChallengeHome();
}

static void trophies(void)
{
	selectWidget("ok", "trophies");

	show = SHOW_TROPHIES;

	initTrophiesDisplay();
}

static void fighterDatabase(void)
{
	show = SHOW_FIGHTER_DB;

	initFighterDatabaseDisplay();
}

static void options(void)
{
	selectWidget("ok", "options");

	show = SHOW_OPTIONS;

	initOptions(returnFromOptions);
}

static void stats(void)
{
	selectWidget("ok", "stats");

	show = SHOW_STATS;

	initStatsDisplay();
}

static void ok(void)
{
	selectWidget("stats", "title");

	show = SHOW_TITLE;
}

static void returnFromOptions(void)
{
	show = SHOW_TITLE;
}

static void credits(void)
{
	initCredits();
}

static void quit(void)
{
	exit(0);
}
