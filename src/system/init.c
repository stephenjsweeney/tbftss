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

#include "init.h"

static void loadConfig(void);
void saveConfig(void);
static void initColor(SDL_Color *c, int r, int g, int b);
static void showLoadingStep(float step, float maxSteps);

void initSDL(void)
{
	int rendererFlags, windowFlags;
	
	/* do this here, so we don't destroy the save dir stored in app */
	memset(&app, 0, sizeof(App));
	
	/* done in src/plat/ */
	createSaveFolder();
	
	rendererFlags = SDL_RENDERER_ACCELERATED;
	windowFlags = 0;
	
	loadConfig();
	
	if (app.vSync)
	{
		rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
	}
	
	if (app.fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}
	
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_ShowCursor(0);
	
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    {
        printf("Couldn't initialize SDL Mixer\n");
		exit(1);
    }
    
    Mix_AllocateChannels(64);
	
	Mix_Volume(-1, app.soundVolume * MIX_MAX_VOLUME / 10);
	Mix_VolumeMusic(app.musicVolume * MIX_MAX_VOLUME / 10);

	app.window = SDL_CreateWindow("TBFTSS - The Pandoran War", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app.winWidth, app.winHeight, windowFlags);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

	if (TTF_Init() < 0)
	{
		printf("Couldn't initialize SDL TTF: %s\n", SDL_GetError());
		exit(1);
	}
	
	app.backBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	app.scaleX = SCREEN_WIDTH;
	app.scaleX /= app.winWidth;
	app.scaleY = SCREEN_HEIGHT;
	app.scaleY /= app.winHeight;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Game scale factor: %.2f,%.2f\n", app.scaleX, app.scaleY);
}

void initGameSystem(void)
{
	int STEPS = 13;
	
	initColor(&colors.red, 255, 0, 0);
	initColor(&colors.orange, 255, 128, 0);
	initColor(&colors.yellow, 255, 255, 0);
	initColor(&colors.green, 0, 255, 0);
	initColor(&colors.blue, 0, 0, 255);
	initColor(&colors.cyan, 0, 255, 255);
	initColor(&colors.purple, 255, 0, 255);
	initColor(&colors.white, 255, 255, 255);
	initColor(&colors.black, 0, 0, 0);
	initColor(&colors.lightGrey, 192, 192, 192);
	initColor(&colors.darkGrey, 128, 128, 128);
	
	showLoadingStep(0, STEPS);
	
	initFonts();
	
	showLoadingStep(1, STEPS);
	
	initInput();
	
	showLoadingStep(2, STEPS);
	
	initLookups();
	
	showLoadingStep(3, STEPS);
	
	initSounds();
	
	showLoadingStep(4, STEPS);
	
	initWidgets();
	
	showLoadingStep(5, STEPS);
	
	initGame();
	
	showLoadingStep(6, STEPS);
	
	loadFighterDefs();
	
	showLoadingStep(7, STEPS);
	
	loadCapitalShipDefs();
	
	showLoadingStep(8, STEPS);
	
	loadItemDefs();
	
	showLoadingStep(9, STEPS);
	
	initBulletDefs();
	
	showLoadingStep(10, STEPS);
	
	initStarSystems();
	
	showLoadingStep(11, STEPS);
	
	initBattle();
	
	showLoadingStep(12, STEPS);
	
	initModalDialog();
	
	showLoadingStep(13, STEPS);
}

/*
 * Just in case the initial loading takes a while on the target machine. The rest of the loading a pretty quick by comparison.
 */
static void showLoadingStep(float step, float maxSteps)
{
	SDL_Rect r;
	
	prepareScene();
	
	r.w = SCREEN_WIDTH - 400;
	r.h = 14;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	r.w *= (step / maxSteps);
	r.x += 2;
	r.y += 2;
	r.w -= 4;
	r.h -= 4;
	
	SDL_SetRenderDrawColor(app.renderer, 128, 196, 255, 255);
	SDL_RenderFillRect(app.renderer, &r);
	
	presentScene();
	
	SDL_Delay(1);
}


static void initColor(SDL_Color *c, int r, int g, int b)
{
	memset(c, 0, sizeof(SDL_Color));
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = 255;
}

static void loadConfig(void)
{
	cJSON *root;
	char *text, *configFilename;
	
	configFilename = getSaveFilePath("config.json");
	
	if (fileExists(configFilename))
	{
		text = readFile(configFilename);
	}
	else
	{
		text = readFile(getFileLocation("data/app/config.json"));
	}
	
	root = cJSON_Parse(text);
	
	app.winWidth = cJSON_GetObjectItem(root, "winWidth")->valueint;
	app.winHeight = cJSON_GetObjectItem(root, "winHeight")->valueint;
	app.vSync = cJSON_GetObjectItem(root, "vSync")->valueint;
	app.fullscreen = cJSON_GetObjectItem(root, "fullscreen")->valueint;
	app.musicVolume = cJSON_GetObjectItem(root, "musicVolume")->valueint;
	app.soundVolume = cJSON_GetObjectItem(root, "soundVolume")->valueint;
	
	cJSON_Delete(root);
	free(text);
	
	/* so that the player doesn't get confused if this is a new game */
	saveConfig();
}

void saveConfig(void)
{
	char *out, *configFilename;
	cJSON *root;
	
	configFilename = getSaveFilePath("config.json");
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Saving config ...");

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "winWidth", app.winWidth);
	cJSON_AddNumberToObject(root, "winHeight", app.winHeight);
	cJSON_AddNumberToObject(root, "vSync", app.vSync);
	cJSON_AddNumberToObject(root, "fullscreen", app.fullscreen);
	cJSON_AddNumberToObject(root, "musicVolume", app.musicVolume);
	cJSON_AddNumberToObject(root, "soundVolume", app.soundVolume);

	out = cJSON_Print(root);

	if (!writeFile(configFilename, out))
	{
		printf("Failed to save config\n");
	}

	cJSON_Delete(root);
	free(out);
}

void cleanup(void)
{
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Cleaning up ...");
	
	SDL_DestroyRenderer(app.renderer);
	SDL_DestroyWindow(app.window);
	
	destroyLookups();
	
	destroyTextures();
	
	expireTexts(1);
	
	destroyFonts();
	
	destroySounds();
	
	destroyGame();
	
	destroyFighterDefs();
	
	destroyCapitalShipDefs();
	
	destroyBulletDefs();
	
	destroyItemDefs();
	
	destroyStarSystems();
	
	destroyBattle();
	
	destroyGalacticMap();
	
	destroyWidgets();

	TTF_Quit();

	SDL_Quit();
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Done");
}
