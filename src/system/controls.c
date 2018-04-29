/*
Copyright (C) 2015-2018 Parallel Realities

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

#include "controls.h"

static void restoreDefaults(void);

static const char *controlName[CONTROL_MAX];
static Widget *controlWidget[CONTROL_MAX];
static char *CONTROLS_TEXT;
static char *HELP_TEXT;
static char *BACKSPACE_TEXT;
static char *ESCAPE_TEXT;

void initControls(void)
{
	int i;
	
	controlName[CONTROL_FIRE] = _("Fire");
	controlName[CONTROL_ACCELERATE] = _("Accelerate");
	controlName[CONTROL_BOOST] = _("Boost");
	controlName[CONTROL_ECM] = _("ECM");
	controlName[CONTROL_BRAKE] = _("Brake");
	controlName[CONTROL_TARGET] = _("Select Target");
	controlName[CONTROL_MISSILE] = _("Fire Missile");
	controlName[CONTROL_GUNS] = _("Cycle Guns");
	controlName[CONTROL_RADAR] = _("Cycle Radar");
	controlName[CONTROL_NEXT_FIGHTER] = _("Next Fighter");
	controlName[CONTROL_PREV_FIGHTER] = _("Previous Fighter");
	controlName[CONTROL_SCREENSHOT] = _("Screenshot");
	
	for (i = 0 ; i < CONTROL_MAX ; i++)
	{
		controlWidget[i] = getWidget(getLookupName("CONTROL_", i), "controls");
		controlWidget[i]->numOptions = 2;
		controlWidget[i]->options = malloc(2 * sizeof(char*));
		controlWidget[i]->options[0] = malloc(sizeof(char) * MAX_NAME_LENGTH);
		controlWidget[i]->options[1] = malloc(sizeof(char) * MAX_NAME_LENGTH);
		strcpy(controlWidget[i]->options[0], "");
		strcpy(controlWidget[i]->options[1], "");
	}
	
	CONTROLS_TEXT = _("Controls");
	HELP_TEXT = _("Click a control to change it, and then the key or mouse button you want to use.");
	BACKSPACE_TEXT = _("[BACKSPACE] - Clear");
	ESCAPE_TEXT = _("[ESCAPE] - Cancel");
}

void initControlsDisplay(void)
{
	int i;
	
	for (i = 0 ; i < CONTROL_MAX ; i++)
	{
		strcpy(controlWidget[i]->options[0], "");
		strcpy(controlWidget[i]->options[1], "");
		
		if (app.keyControls[i] != 0)
		{
			sprintf(controlWidget[i]->options[0], "%s", SDL_GetScancodeName(app.keyControls[i]));
		}
		
		if (app.mouseControls[i] != 0)
		{
			sprintf(controlWidget[i]->options[1], "Btn %d", app.mouseControls[i]);
		}
	}
	
	getWidget("restore", "controls")->action = restoreDefaults;
}

int isControl(int type)
{
	int key = app.keyControls[type];
	int btn = app.mouseControls[type];
	
	return ((key != 0 && app.keyboard[key]) || (btn != 0 && app.mouse.button[btn]));
}

int isAcceptControl(void)
{
	return (app.keyboard[SDL_SCANCODE_SPACE] ||app.keyboard[SDL_SCANCODE_RETURN] || isControl(CONTROL_FIRE));
}

void clearControl(int type)
{
	int key = app.keyControls[type];
	int btn = app.mouseControls[type];
	
	if (key != 0)
	{
		app.keyboard[key] = 0;
	}
	
	if (btn != 0)
	{
		app.mouse.button[btn] = 0;
	}
}

void resetAcceptControls(void)
{
	app.keyboard[SDL_SCANCODE_SPACE] = app.keyboard[SDL_SCANCODE_RETURN] = 0;
	
	clearControl(CONTROL_FIRE);
}

void updateControlKey(char *name)
{
	app.keyControls[lookup(name)] = app.lastKeyPressed;
	
	initControlsDisplay();
}

void updateControlButton(char *name)
{
	app.mouseControls[lookup(name)] = app.lastButtonPressed;
	
	initControlsDisplay();
}

void clearControlConfig(char *name)
{
	int i;
	
	i = lookup(name);
	
	app.keyControls[i] = app.mouseControls[i] = 0;
	
	initControlsDisplay();
}

void drawControls(void)
{
	int i;
	SDL_Rect r;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

	r.w = 800;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);

	drawText(SCREEN_WIDTH / 2, 50, 28, TA_CENTER, colors.white, CONTROLS_TEXT);

	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, r.y + 65, r.x + r.w, r.y + 65);
	
	r.x += 25;
	r.y = 125;
	
	for (i = 0 ; i < CONTROL_MAX ; i++)
	{
		drawText(r.x, r.y + 2, 20, TA_LEFT, colors.white, controlName[i]);
		
		controlWidget[i]->rect.x = r.x + 175;
		controlWidget[i]->rect.y = r.y;
		
		r.y += 65;
		
		if (r.y > 500)
		{
			r.y = 125;
			r.x += 400;
		}
	}
	
	limitTextWidth(r.w - 100);
	drawText(SCREEN_WIDTH / 2, 525, 16, TA_CENTER, colors.white, HELP_TEXT);
	drawText((SCREEN_WIDTH / 2) - 50, 560, 16, TA_RIGHT, colors.white, BACKSPACE_TEXT);
	drawText((SCREEN_WIDTH / 2) + 50, 560, 16, TA_LEFT, colors.white, ESCAPE_TEXT);
	limitTextWidth(0);
	
	drawWidgets("controls");
}

static void restoreDefaults(void)
{
	int i;
	cJSON *root, *controlsJSON, *node;
	char *text;

	text = readFile("data/app/"CONFIG_FILENAME);

	root = cJSON_Parse(text);

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

	cJSON_Delete(root);
	free(text);
	
	initControlsDisplay();
}
