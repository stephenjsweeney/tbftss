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

#include "options.h"

static void changeWindowSize(char *value);
static void changeSoundVolume(char *value);
static void changeMusicVolume(char *value);
static void changeVSync(char *value);
static void changeFullscreen(char *value);
static void ok(void);

static void (*returnFromOptions)(void);

void initOptions(void (*rtn)(void))
{
	char optionStr[MAX_NAME_LENGTH];
	
	selectWidget("windowSize", "options");
	
	getWidget("windowSize", "options")->onChange = changeWindowSize;
	getWidget("soundVolume", "options")->onChange = changeSoundVolume;
	getWidget("musicVolume", "options")->onChange = changeMusicVolume;
	getWidget("vSync", "options")->onChange = changeVSync;
	getWidget("fullscreen", "options")->onChange = changeFullscreen;
	getWidget("ok", "options")->action = ok;
	
	sprintf(optionStr, "%d x %d", app.winWidth, app.winHeight);
	setWidgetOption("windowSize", "options", optionStr);
	
	sprintf(optionStr, "%d", app.soundVolume);
	setWidgetOption("soundVolume", "options", optionStr);
	
	sprintf(optionStr, "%d", app.musicVolume);
	setWidgetOption("musicVolume", "options", optionStr);
	
	setWidgetOption("vSync", "options", app.vSync ? "On" : "Off");
	
	setWidgetOption("fullscreen", "options", app.fullscreen ? "On" : "Off");
	
	returnFromOptions = rtn;
}

void drawOptions(void)
{
	SDL_Rect r;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = 500;
	r.h = 600;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 70, 28, TA_CENTER, colors.white, _("Options"));
	
	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, 120, r.x + r.w, 120);
		
	drawWidgets("options");	
	
	limitTextWidth(r.w - 100);
	drawText(SCREEN_WIDTH / 2, r.y + r.h - 135, 16, TA_CENTER, colors.yellow, _("Note: you must restart the game for window size and fullscreen options to take effect."));
	limitTextWidth(0);
}

static void changeWindowSize(char *value)
{
	sscanf(value, "%d x %d", &app.winWidth, &app.winHeight);
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

static void changeVSync(char *value)
{
	app.vSync = strcmp(value, "On") == 0;
}

static void changeFullscreen(char *value)
{
	app.fullscreen = strcmp(value, "On") == 0;
}

static void ok(void)
{
	saveConfig();
	
	returnFromOptions();
}
