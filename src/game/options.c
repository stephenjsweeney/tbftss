/*
Copyright (C) 2015-2019 Parallel Realities

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

#include "options.h"

static void changeWindowSize(char *value);
static void changeSoundVolume(char *value);
static void changeMusicVolume(char *value);
static void changeFullscreen(char *value);
static void ok(void);
static void controlsOK(void);
static void drawMain(void);
static void controls(void);

static void (*returnFromOptions)(void);
static int show;
static char *OPTIONS_TEXT;
static char *RESOLUTION_TEXT;

void initOptions(void (*rtn)(void))
{
	char optionStr[MAX_NAME_LENGTH];

	selectWidget("windowSize", "options");

	getWidget("windowSize", "options")->onChange = changeWindowSize;
	getWidget("soundVolume", "options")->onChange = changeSoundVolume;
	getWidget("musicVolume", "options")->onChange = changeMusicVolume;
	getWidget("fullscreen", "options")->onChange = changeFullscreen;
	getWidget("controls", "options")->action = controls;
	getWidget("ok", "options")->action = ok;
	getWidget("ok", "controls")->action = controlsOK;

	sprintf(optionStr, "%d x %d", app.winWidth, app.winHeight);
	setWidgetOption("windowSize", "options", optionStr);

	sprintf(optionStr, "%d", app.soundVolume);
	setWidgetOption("soundVolume", "options", optionStr);

	sprintf(optionStr, "%d", app.musicVolume);
	setWidgetOption("musicVolume", "options", optionStr);

	setWidgetOption("fullscreen", "options", app.fullscreen ? "On" : "Off");
	
	OPTIONS_TEXT = _("Options");
	RESOLUTION_TEXT = _("Note: you must restart the game for window size and fullscreen options to take effect.");

	#if FIXED_RESOLUTION
	getWidget("windowSize", "options")->enabled = 0;
	getWidget("fullscreen", "options")->enabled = 0;
	RESOLUTION_TEXT = _("Note: this device does not support changing the screen resolution.");
	#endif

	returnFromOptions = rtn;
	
	show = SHOW_MAIN;
}

void drawOptions(void)
{
	switch (show)
	{
		case SHOW_MAIN:
			drawMain();
			break;
			
		case SHOW_CONTROLS:
			drawControls();
			break;
	}
}

static void drawMain(void)
{
	SDL_Rect r;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	SDL_SetRenderTarget(app.renderer, app.uiBuffer);

	r.w = 500;
	r.h = 600;
	r.x = (UI_WIDTH / 2) - r.w / 2;
	r.y = (UI_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);

	drawText(UI_WIDTH / 2, 70, 28, TA_CENTER, colors.white, OPTIONS_TEXT);

	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, 120, r.x + r.w, 120);

	drawWidgets("options");

	app.textWidth = r.w - 100;
	
	drawText(UI_WIDTH / 2, r.y + r.h - 135, 16, TA_CENTER, colors.yellow, RESOLUTION_TEXT);
	
	app.textWidth = 0;
	
	SDL_SetRenderTarget(app.renderer, app.backBuffer);
}

void updateCustomResolutionOption(void)
{
	Widget *w;
	char value[MAX_NAME_LENGTH];
	int i;
	
	sprintf(value, "%d x %d", app.winWidth, app.winHeight);
	
	w = getWidget("windowSize", "options");
	
	for (i = 0 ; i < w->numOptions - 1 ; i++)
	{
		if (strcmp(w->options[i], value) == 0)
		{
			w->numOptions--;
			return;
		}
	}
	
	w->options[w->numOptions - 1] = malloc(strlen(value) + 1);
	strcpy(w->options[w->numOptions - 1], value);
}

static void controls(void)
{
	initControlsDisplay();
	
	show = SHOW_CONTROLS;
}

static void changeWindowSize(char *value)
{
	sscanf(value, "%d x %d", &app.winWidth, &app.winHeight);
	
	SDL_SetWindowSize(app.window, app.winWidth, app.winHeight);
	
	app.uiOffset.x = (app.winWidth / 2) - (UI_WIDTH / 2);
	app.uiOffset.y = (app.winHeight / 2) - (UI_HEIGHT / 2);
	
	SDL_DestroyTexture(app.backBuffer);
	
	app.backBuffer = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, app.winWidth, app.winHeight);
	
	initGraphics();
	
	initStars();
}

static void changeSoundVolume(char *value)
{
	app.soundVolume = atoi(value);

	Mix_Volume(-1, app.soundVolume * MIX_MAX_VOLUME / 10);
}

static void changeMusicVolume(char *value)
{
	app.musicVolume = atoi(value);

	Mix_VolumeMusic(app.musicVolume * MIX_MAX_VOLUME / 10);
}

static void changeFullscreen(char *value)
{
	app.fullscreen = strcmp(value, "On") == 0;
	
	SDL_SetWindowFullscreen(app.window, app.fullscreen? SDL_WINDOW_FULLSCREEN : 0);
}

static void ok(void)
{
	saveConfig();

	returnFromOptions();
}

static void controlsOK(void)
{
	show = SHOW_MAIN;
}
