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

#include "title.h"

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void initFighters(void);
static void doFighters(void);
static void drawFighters(void);
static void campaign(void);
static void challenges(void);
static void trophies(void);
static void stats(void);
static void ok(void);
static void options(void);
static void credits(void);
static void quit(void);
static void returnFromOptions(void);
static void newCampaign(void);
static void easyCampaign(void);
static void continueCampaign(void);
static void back(void);
static void doEasyCampaign(void);
static void doNewCampaign(void);
static void ignoreNewCampaign(void);
static void drawEasyModeHints(void);

static SDL_Texture *background;
static SDL_Texture *logo;
static SDL_Texture *pandoranWar;
static SDL_Texture *earthTexture;
static PointF earth;
static Entity fighters[NUM_FIGHTERS];
static const char *fighterTextures[] = {"gfx/fighters/firefly.png", "gfx/fighters/hammerhead.png", "gfx/fighters/hyena.png", "gfx/fighters/lynx.png", "gfx/fighters/kingfisher.png", "gfx/fighters/leopard.png", "gfx/fighters/nymph.png", "gfx/fighters/ray.png", "gfx/fighters/rook.png", "gfx/fighters/taf.png"};
static int show;
static int hasActiveCampaign;

void initTitle(void)
{
	startSectionTransition();
	
	stopMusic();
	
	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	battle.camera.x =  battle.camera.y = 0;
	
	destroyBattle();
	
	logo = getTexture("gfx/title/logo.png");
	
	pandoranWar = getTexture("gfx/title/pandoran.png");
	
	background = getTexture("gfx/backgrounds/background02.jpg");
	
	earthTexture = getTexture("gfx/planets/earth.png");
	
	earth.x = rand() % SCREEN_WIDTH;
	earth.y = -(128 + (rand() % 128));
	
	initBackground();
	
	initEffects();
	
	initFighters();
	
	updateAllMissions();
	
	hasActiveCampaign = hasCompletedAnyMission();
	
	getWidget("campaign", "title")->action = campaign;
	getWidget("challenges", "title")->action = challenges;
	getWidget("trophies", "title")->action = trophies;
	getWidget("stats", "title")->action = stats;
	getWidget("options", "title")->action = options;
	getWidget("credits", "title")->action = credits;
	getWidget("quit", "title")->action = quit;
	
	getWidget("new", "campaign")->action = newCampaign;
	getWidget("easy", "campaign")->action = easyCampaign;
	getWidget("continue", "campaign")->action = continueCampaign;
	getWidget("continue", "campaign")->enabled = hasActiveCampaign;
	getWidget("back", "campaign")->action = back;
	
	getWidget("ok", "stats")->action = ok;
	getWidget("ok", "trophies")->action = ok;
	
	show = SHOW_TITLE;
	
	endSectionTransition();
	
	playMusic("music/main/Rise of spirit.ogg", 0);
}

static void initFighters(void)
{
	int i, numTextures;
	
	numTextures = sizeof(fighterTextures) / sizeof(char*);
	
	memset(&fighters, 0, sizeof(Entity) * NUM_FIGHTERS);
	
	for (i = 0 ; i < NUM_FIGHTERS ; i++)
	{
		fighters[i].x = rand() % (SCREEN_WIDTH - 32);
		fighters[i].y = SCREEN_HEIGHT + (rand() % SCREEN_HEIGHT);
		fighters[i].texture = getTexture(fighterTextures[rand() % numTextures]);
		fighters[i].dy = -(1 + rand() % 3);
	}
}

static void logic(void)
{
	handleKeyboard();
	
	scrollBackground(0, 0.25);
	
	doStars(0, -0.5);
	
	earth.y += 0.1;
	
	if (earth.y > SCREEN_HEIGHT + 128)
	{
		earth.x = rand() % SCREEN_WIDTH;
		earth.y = -(128 + (rand() % 128));
	}
	
	doFighters();
	
	doEffects();
	
	app.doTrophyAlerts = 1;
	
	doWidgets();
}

static void doFighters(void)
{
	int i, numTextures;
	
	numTextures = sizeof(fighterTextures) / sizeof(char*);
	
	for (i = 0 ; i < NUM_FIGHTERS ; i++)
	{
		self = &fighters[i];
		
		/* engine position hack, due to camera being fixed */
		self->y += 16;
		addEngineEffect();
		self->y -= 16;
		
		self->y += self->dy;
		
		if (self->y <= -64)
		{
			self->x = rand() % (SCREEN_WIDTH - 32);
			self->y = SCREEN_HEIGHT + (rand() % SCREEN_HEIGHT);
			self->texture = getTexture(fighterTextures[rand() % numTextures]);
			self->dy = -(1 + rand() % 3);
		}
	}
}

static void draw(void)
{
	drawBackground(background);
	
	drawStars();
	
	blit(earthTexture, earth.x, earth.y, 1);
	
	drawFighters();
	
	drawEffects();
	
	blit(logo, SCREEN_WIDTH / 2, 50, 1);
	
	blit(pandoranWar, SCREEN_WIDTH / 2, 110, 1);
	
	drawText(10, SCREEN_HEIGHT - 25, 14, TA_LEFT, colors.white, "Copyright Parallel Realities, 2015-2017");
	drawText(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 25, 14, TA_RIGHT, colors.white, "Version %.2f-%d", VERSION, REVISION);
	
	switch (show)
	{
		case SHOW_TITLE:
			drawWidgets("title");
			break;
			
		case SHOW_CAMPAIGN:
			drawWidgets("campaign");
			drawEasyModeHints();
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
	}
}

static void drawFighters(void)
{
	int i;
	
	for (i = 0 ; i < NUM_FIGHTERS ; i++)
	{
		blit(fighters[i].texture, fighters[i].x, fighters[i].y, 1);
	}
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE] && !app.awaitingWidgetInput)
	{
		returnFromOptions();
		playSound(SND_GUI_CLOSE);
		
		clearInput();
	}
}

static void campaign(void)
{
	show = SHOW_CAMPAIGN;
}

static void drawEasyModeHints(void)
{
	drawText(SCREEN_WIDTH / 2, 525, 16, TA_CENTER, colors.white, _("Easy Campaign features reduced AI aggression, as well as reduced Boost and ECM recharge times."));
	drawText(SCREEN_WIDTH / 2, 550, 16, TA_CENTER, colors.white, _("The player also benefits from increased fighter armour and shield recharge rates."));
	drawText(SCREEN_WIDTH / 2, 575, 16, TA_CENTER, colors.white, _("Some players may find this mode more accessible than the normal campaign difficulty."));
	drawText(SCREEN_WIDTH / 2, 600, 16, TA_CENTER, colors.white, _("Note that the difficulty cannot be changed once the campaign is started."));
}

static void newCampaign(void)
{
	if (!hasActiveCampaign)
	{
		doNewCampaign();
	}
	else
	{
		showOKCancelDialog(&doNewCampaign, &ignoreNewCampaign, _("Start a new campaign? Your current campaign progress will be lost."));
	}
}

static void doNewCampaign(void)
{
	app.modalDialog.type = MD_NONE;
	
	resetCampaign(DIFFICULTY_NORMAL);
	
	destroyBattle();
	
	initGalacticMap();
}

static void easyCampaign(void)
{
	if (!hasActiveCampaign)
	{
		doEasyCampaign();
	}
	else
	{
		showOKCancelDialog(&doEasyCampaign, &ignoreNewCampaign, _("Start a new campaign? Your current campaign progress will be lost."));
	}
}

static void doEasyCampaign(void)
{
	app.modalDialog.type = MD_NONE;
	
	resetCampaign(DIFFICULTY_EASY);
	
	destroyBattle();
	
	initGalacticMap();
}

static void continueCampaign(void)
{
	app.modalDialog.type = MD_NONE;
	
	destroyBattle();
	
	initGalacticMap();
}

static void ignoreNewCampaign(void)
{
	app.modalDialog.type = MD_NONE;
}

static void back(void)
{
	show = SHOW_TITLE;
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
