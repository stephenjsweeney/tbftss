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
static void newGame(void);
static void continueGame(void);
static void options(void);
static void quit(void);
static void returnFromOptions(void);

static SDL_Texture *background;
static SDL_Texture *logo;
static SDL_Texture *pandoranWar;
static SDL_Texture *earthTexture;
static PointF earth;
static Entity fighters[NUM_FIGHTERS];
static const char *fighterTextures[] = {"gfx/fighters/firefly.png", "gfx/fighters/hammerhead.png", "gfx/fighters/hyena.png", "gfx/fighters/khepri.png", "gfx/fighters/kingfisher.png", "gfx/fighters/leopard.png", "gfx/fighters/nymph.png", "gfx/fighters/ray.png", "gfx/fighters/rook.png", "gfx/fighters/taf.png"};
static int showingOptions;

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
	
	if (fileExists(getSaveFilePath("game.save")))
	{
		selectWidget("continue", "title");
	}
	else
	{
		getWidget("continue", "title")->enabled = 0;
		
		selectWidget("newGame", "title");
	}
	
	getWidget("newGame", "title")->action = newGame;
	getWidget("continue", "title")->action = continueGame;
	getWidget("options", "title")->action = options;
	getWidget("quit", "title")->action = quit;
	
	showingOptions = 0;
	
	endSectionTransition();
	
	SDL_WarpMouseInWindow(app.window, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 100);
	
	playMusic("music/Rise of spirit.ogg");
}

static void initFighters(void)
{
	int i, numTextures;
	
	numTextures = sizeof(fighterTextures) / sizeof(char*);
	
	memset(&fighters, 0, sizeof(Entity) * NUM_FIGHTERS);
	
	for (i = 0 ; i < NUM_FIGHTERS ; i++)
	{
		fighters[i].x = rand() % (SCREEN_WIDTH - 64);
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
	
	doWidgets();
}

static void doFighters(void)
{
	int i;
	
	for (i = 0 ; i < NUM_FIGHTERS ; i++)
	{
		fighters[i].y += fighters[i].dy;
		
		self = &fighters[i];
			
		addEngineEffect();
		
		if (fighters[i].y <= -64)
		{
			fighters[i].x = rand() % (SCREEN_WIDTH - 64);
			fighters[i].y = SCREEN_HEIGHT + (rand() % SCREEN_HEIGHT);
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
	
	drawText(10, SCREEN_HEIGHT - 25, 14, TA_LEFT, colors.white, "Copyright Parallel Realities, 2015-2016");
	drawText(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 25, 14, TA_RIGHT, colors.white, "Version %.2f.%d", VERSION, REVISION);
	
	if (!showingOptions)
	{
		drawWidgets("title");
	}
	else
	{
		drawOptions();
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
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		returnFromOptions();
		
		memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
		
		playSound(SND_GUI_CLOSE);
	}
}

static void newGame(void)
{
	resetGame();
	
	initGalacticMap();
}

static void continueGame(void)
{
	loadGame();
	
	initGalacticMap();
}

static void options(void)
{
	showingOptions = 1;
	
	initOptions(returnFromOptions);
}

static void returnFromOptions(void)
{
	showingOptions = 0;
	
	selectWidget("newGame", "title");
}

static void quit(void)
{
	exit(1);
}
