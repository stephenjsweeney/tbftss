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

#include <locale.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "../common.h"

#include "../battle/battle.h"
#include "../battle/bullets.h"
#include "../battle/capitalShips.h"
#include "../battle/fighters.h"
#include "../battle/items.h"
#include "../battle/starfield.h"
#include "../challenges/challenges.h"
#include "../galaxy/galacticMap.h"
#include "../galaxy/starSystems.h"
#include "../game/credits.h"
#include "../game/fighterDatabase.h"
#include "../game/game.h"
#include "../game/options.h"
#include "../game/stats.h"
#include "../game/trophies.h"
#include "../json/cJSON.h"
#include "../plat/win32/win32Init.h"
#include "../system/atlas.h"
#include "../system/controls.h"
#include "../system/draw.h"
#include "../system/i18n.h"
#include "../system/input.h"
#include "../system/io.h"
#include "../system/lookup.h"
#include "../system/modalDialog.h"
#include "../system/resources.h"
#include "../system/sound.h"
#include "../system/text.h"
#include "../system/textures.h"
#include "../system/util.h"
#include "../system/widgets.h"
#include "init.h"

extern App app;

static void loadConfig(int argc, char *argv[]);
static void loadConfigFile(char *filename);
static void showLoadingStep(float step, float maxSteps);
static void handleCommandLineConfig(int argc, char *argv[]);

void init18N(int argc, char *argv[])
{
	int i;
	int languageId = -1;

	setlocale(LC_NUMERIC, "");

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-language") == 0)
		{
			languageId = i + 1;

			if (languageId >= argc)
			{
				SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "You must specify a language to use with -language. Using default.");
			}
		}
	}

	setLanguage("tbftss", languageId == -1 ? NULL : argv[languageId]);

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Numeric is %s", setlocale(LC_NUMERIC, "C"));
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "atof(2.75) is %f", atof("2.75"));
}

void initSDL(int argc, char *argv[])
{
	int rendererFlags, windowFlags;
	int i;

	/* do this here, so we don't destroy the save dir stored in app */
	memset(&app, 0, sizeof(App));

	/* done in src/plat/ */
	createSaveFolder();

	loadConfig(argc, argv);

	rendererFlags = SDL_RENDERER_ACCELERATED;

	if (app.vSync)
	{
		rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
	}

	windowFlags = 0;

	if (app.fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_ShowCursor(0);

	if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNKSIZE) == -1)
	{
		printf("Couldn't initialize SDL Mixer\n");
		exit(1);
	}

	Mix_AllocateChannels(AUDIO_MIX_CHANNELS);

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
	for (i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			app.controllerIndex = i;
			app.controller = SDL_GameControllerOpen(i);
			break;
		}
	}

	app.backBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, app.winWidth, app.winHeight);

	app.uiBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, UI_WIDTH, UI_HEIGHT);
	SDL_SetTextureBlendMode(app.uiBuffer, SDL_BLENDMODE_BLEND);

	app.uiOffset.x = (app.winWidth / 2) - (UI_WIDTH / 2);
	app.uiOffset.y = (app.winHeight / 2) - (UI_HEIGHT / 2);

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "UI offset: %d,%d\n", app.uiOffset.x, app.uiOffset.y);
}

void initGameSystem(void)
{
	int i, numInitFuns;
	void (*initFuncs[])(void) = {initGraphics, initFonts, initResources, initSounds, initWidgets, initGame, loadFighterDefs, loadCapitalShipDefs, loadItemDefs, initBulletDefs, initStarSystems, initChallenges, initStats, initModalDialog, initStars, initControls, initTrophies, initFighterDatabase, updateCustomResolutionOption};

	initAtlas();

	initInput();

	numInitFuns = sizeof(initFuncs) / sizeof(void *);

	for (i = 0; i < numInitFuns; i++)
	{
		showLoadingStep(i + 1, numInitFuns);

		initFuncs[i]();
	}
}

/*
 * Just in case the initial loading takes a while on the target machine. The rest of the loading is pretty quick by comparison.
 */
static void showLoadingStep(float step, float maxSteps)
{
	SDL_Rect r;

	prepareScene();

	r.w = 600;
	r.h = 12;
	r.x = (app.winWidth / 2) - r.w / 2;
	r.y = (app.winHeight / 2) - r.h / 2;

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

static void loadConfig(int argc, char *argv[])
{
	char *configFilename;

	/* load default config first */
	loadConfigFile("data/app/" CONFIG_FILENAME);

	/* load saved config */
	configFilename = getSaveFilePath(CONFIG_FILENAME);

	if (fileExists(configFilename))
	{
		loadConfigFile(configFilename);
	}

	handleCommandLineConfig(argc, argv);

	/* don't go higher than 8K or lower than 1280 x 720 */
	app.winWidth = MIN(MAX(app.winWidth, 1280), 7680);
	app.winHeight = MIN(MAX(app.winHeight, 720), 4320);

	/* so that the player doesn't get confused if this is a new game */
	saveConfig();
}

static void loadConfigFile(char *filename)
{
	int    i;
	cJSON *root, *controlsJSON, *node, *gameplayJSON;
	char  *text;

	text = readFile(filename);

	root = cJSON_Parse(text);

	app.winWidth = cJSON_GetObjectItem(root, "winWidth")->valueint;
	app.winHeight = cJSON_GetObjectItem(root, "winHeight")->valueint;
	app.fullscreen = cJSON_GetObjectItem(root, "fullscreen")->valueint;
	app.musicVolume = cJSON_GetObjectItem(root, "musicVolume")->valueint;
	app.soundVolume = cJSON_GetObjectItem(root, "soundVolume")->valueint;
	app.vSync = getJSONValue(root, "vSync", 1);

	controlsJSON = cJSON_GetObjectItem(root, "controls");
	if (controlsJSON)
	{
		node = cJSON_GetObjectItem(controlsJSON, "keys")->child;
		while (node)
		{
			i = lookup(node->string);

			app.keyControls[i] = node->valueint;

			node = node->next;
		}

		node = cJSON_GetObjectItem(controlsJSON, "mouse")->child;
		while (node)
		{
			i = lookup(node->string);

			app.mouseControls[i] = node->valueint;

			node = node->next;
		}
	}
	app.controllerControls[CONTROL_FIRE][0] = 1;
	app.controllerControls[CONTROL_FIRE][1] = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
	app.controllerControls[CONTROL_ACCELERATE][0] = 0;
	app.controllerControls[CONTROL_ACCELERATE][1] = SDL_CONTROLLER_BUTTON_A;
	app.controllerControls[CONTROL_BOOST][0] = 0;
	app.controllerControls[CONTROL_BOOST][1] = SDL_CONTROLLER_BUTTON_B;
	app.controllerControls[CONTROL_ECM][0] = 0;
	app.controllerControls[CONTROL_ECM][1] = SDL_CONTROLLER_BUTTON_X;
	app.controllerControls[CONTROL_BRAKE][0] = 0;
	app.controllerControls[CONTROL_BRAKE][1] = SDL_CONTROLLER_BUTTON_Y;
	app.controllerControls[CONTROL_TARGET][0] = 1;
	app.controllerControls[CONTROL_TARGET][1] = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
	app.controllerControls[CONTROL_MISSILE][0] = 0;
	app.controllerControls[CONTROL_MISSILE][1] = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
	app.controllerControls[CONTROL_GUNS][0] = 0;
	app.controllerControls[CONTROL_GUNS][1] = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
	app.controllerControls[CONTROL_RADAR][0] = 0;
	app.controllerControls[CONTROL_RADAR][1] = SDL_CONTROLLER_BUTTON_DPAD_UP;
	app.controllerControls[CONTROL_NEXT_FIGHTER][0] = 0;
	app.controllerControls[CONTROL_NEXT_FIGHTER][1] = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
	app.controllerControls[CONTROL_PREV_FIGHTER][0] = 0;
	app.controllerControls[CONTROL_PREV_FIGHTER][1] = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
	app.controllerControls[CONTROL_SCREENSHOT][0] = 0;
	app.controllerControls[CONTROL_SCREENSHOT][1] = SDL_CONTROLLER_BUTTON_BACK;

	gameplayJSON = cJSON_GetObjectItem(root, "gameplay");
	if (gameplayJSON)
	{
		app.gameplay.friendlyFire = cJSON_GetObjectItem(gameplayJSON, "friendlyFire")->valueint;
		app.gameplay.autoSwitchPlayerTarget = cJSON_GetObjectItem(gameplayJSON, "autoSwitchPlayerTarget")->valueint;
		app.gameplay.missileReTarget = cJSON_GetObjectItem(gameplayJSON, "missileReTarget")->valueint;
		app.gameplay.healthBars = cJSON_GetObjectItem(gameplayJSON, "healthBars")->valueint;
	}

	cJSON_Delete(root);
	free(text);
}

void saveConfig(void)
{
	int    i;
	char  *out, *configFilename;
	cJSON *root, *controlsJSON, *keysJSON, *mouseJSON, *gameplayJSON;

	configFilename = getSaveFilePath(CONFIG_FILENAME);

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Saving config ...");

	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "winWidth", app.winWidth);
	cJSON_AddNumberToObject(root, "winHeight", app.winHeight);
	cJSON_AddNumberToObject(root, "fullscreen", app.fullscreen);
	cJSON_AddNumberToObject(root, "musicVolume", app.musicVolume);
	cJSON_AddNumberToObject(root, "soundVolume", app.soundVolume);
	cJSON_AddNumberToObject(root, "vSync", app.vSync);

	keysJSON = cJSON_CreateObject();
	for (i = 0; i < CONTROL_MAX; i++)
	{
		cJSON_AddNumberToObject(keysJSON, getLookupName("CONTROL_", i), app.keyControls[i]);
	}

	mouseJSON = cJSON_CreateObject();
	for (i = 0; i < CONTROL_MAX; i++)
	{
		cJSON_AddNumberToObject(mouseJSON, getLookupName("CONTROL_", i), app.mouseControls[i]);
	}

	controlsJSON = cJSON_CreateObject();
	cJSON_AddItemToObject(controlsJSON, "keys", keysJSON);
	cJSON_AddItemToObject(controlsJSON, "mouse", mouseJSON);
	cJSON_AddItemToObject(root, "controls", controlsJSON);

	gameplayJSON = cJSON_CreateObject();
	cJSON_AddNumberToObject(gameplayJSON, "friendlyFire", app.gameplay.friendlyFire);
	cJSON_AddNumberToObject(gameplayJSON, "autoSwitchPlayerTarget", app.gameplay.autoSwitchPlayerTarget);
	cJSON_AddNumberToObject(gameplayJSON, "missileReTarget", app.gameplay.missileReTarget);
	cJSON_AddNumberToObject(gameplayJSON, "healthBars", app.gameplay.healthBars);
	cJSON_AddItemToObject(root, "gameplay", gameplayJSON);

	out = cJSON_Print(root);

	if (!writeFile(configFilename, out))
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Failed to save config");
	}

	cJSON_Delete(root);
	free(out);
}

static void handleCommandLineConfig(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-size") == 0)
		{
			sscanf(argv[i + 1], "%dx%d", &app.winWidth, &app.winHeight);
		}
	}
}

void cleanup(void)
{
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Cleaning up ...");

	destroyLookups();

	destroyTextures();

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

	destroyResources();

	destroyFighterDatabase();

	destroyFighterStats();

	destroyCredits();

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Done");

	SDL_DestroyRenderer(app.renderer);

	SDL_DestroyWindow(app.window);

	TTF_Quit();

	SDL_Quit();
}
