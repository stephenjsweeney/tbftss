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

#include "../system/atlas.h"
#include "../system/draw.h"
#include "../system/sound.h"
#include "input.h"

extern App app;

static AtlasImage *mousePointer;
static AtlasImage *mousePointerNormal;
static AtlasImage *mousePointerMove;

void initInput(void)
{
	memset(&app.mouse, 0, sizeof(Mouse));

	mousePointerNormal = getAtlasImage("gfx/input/mousePointer.png");
	mousePointerMove = getAtlasImage("gfx/input/mousePointerMove.png");

	mousePointer = mousePointerNormal;
	app.mouse.w = mousePointer->rect.w;
	app.mouse.h = mousePointer->rect.h;
}

void doKeyDown(SDL_KeyboardEvent *event)
{
	if (event->keysym.scancode < MAX_KEYBOARD_KEYS && event->repeat == 0)
	{
		app.keyboard[event->keysym.scancode] = 1;
		app.lastKeyPressed = event->keysym.scancode;
	}
}

void doKeyUp(SDL_KeyboardEvent *event)
{
	if (event->keysym.scancode < MAX_KEYBOARD_KEYS)
	{
		app.keyboard[event->keysym.scancode] = 0;
	}
}

void doMouseDown(SDL_MouseButtonEvent *event)
{
	if (event->button >= 0 && event->button < MAX_MOUSE_BUTTONS)
	{
		app.mouse.button[event->button] = 1;
		app.lastButtonPressed = event->button;
	}
}

void doMouseUp(SDL_MouseButtonEvent *event)
{
	if (event->button >= 0 && event->button < MAX_MOUSE_BUTTONS)
	{
		app.mouse.button[event->button] = 0;
	}
}

/*
 * Note: the following assumes that SDL_BUTTON_X1 and SDL_BUTTON_X2 are 4 and 5, respectively. They usually are.
 */
void doMouseWheel(SDL_MouseWheelEvent *event)
{
	if (event->y == -1)
	{
		app.mouse.button[SDL_BUTTON_X1] = 1;
		app.lastButtonPressed = SDL_BUTTON_X1;
	}

	if (event->y == 1)
	{
		app.mouse.button[SDL_BUTTON_X2] = 1;
		app.lastButtonPressed = SDL_BUTTON_X2;
	}
}

void doMouseMotion(SDL_MouseMotionEvent *event)
{
	app.mouse.dx = event->xrel;
	app.mouse.dy = event->yrel;
	app.controllerX = CONTROLLER_NOINPUT;
	app.controllerY = CONTROLLER_NOINPUT;
}

void setMouseCursor(int isDrag)
{
	if (isDrag)
	{
		mousePointer = mousePointerMove;
	}
	else
	{
		mousePointer = mousePointerNormal;
	}
}

void drawMouse(void)
{
	setAtlasColor(255, 255, 255, 255);

	if (app.controllerX != CONTROLLER_NOINPUT)
	{
		blit(mousePointer, app.controllerX, app.controllerY, 1);
	}
	else
	{
		blit(mousePointer, app.mouse.x, app.mouse.y, 1);
	}
}

void doControllerAxis(SDL_ControllerAxisEvent *event)
{
	int i;
	if (event->which == app.controllerIndex)
	{
		if (event->axis == SDL_CONTROLLER_AXIS_LEFTX)
		{
			app.controllerAxis[0] = event->value / 1024;
		}
		else if (event->axis == SDL_CONTROLLER_AXIS_LEFTY)
		{
			app.controllerAxis[1] = event->value / 1024;
		}
		else if (event->axis == SDL_CONTROLLER_AXIS_RIGHTX)
		{
			app.controllerAxis[2] = event->value / 1024 / 4;
		}
		else if (event->axis == SDL_CONTROLLER_AXIS_RIGHTY)
		{
			app.controllerAxis[3] = event->value / 1024 / 4;
		}
		else
		{
			for (i = 0; i < CONTROL_MAX; i++)
			{
				if (app.controllerControls[i][0] == 1 && app.controllerControls[i][1] == event->axis)
				{
					if (event->value > 3200)
					{
						app.controllerButton[i] = 1;
					}
					else
					{
						app.controllerButton[i] = 0;
					}
				}
			}
		}
	}
}

void doControllerButtonDown(SDL_ControllerButtonEvent *event)
{
	int i;
	if (event->which == app.controllerIndex)
	{
		for (i = 0; i < CONTROL_MAX; i++)
		{
			if (app.controllerControls[i][0] == 0 && app.controllerControls[i][1] == event->button)
			{
				app.controllerButton[i] = 1;
				break;
			}
		}
		if (event->button == SDL_CONTROLLER_BUTTON_START)
		{
			app.controllerStart = 1;
		}
	}
}

void doControllerButtonUp(SDL_ControllerButtonEvent *event)
{
	int i;
	if (event->which == app.controllerIndex)
	{
		for (i = 0; i < CONTROL_MAX; i++)
		{
			if (app.controllerControls[i][0] == 0 && app.controllerControls[i][1] == event->button)
			{
				app.controllerButton[i] = 0;
				break;
			}
		}
		if (event->button == SDL_CONTROLLER_BUTTON_START)
		{
			app.controllerStart = 0;
		}
	}
}

void doInput(void)
{
	SDL_Event event;
	int       x, y;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_MOUSEMOTION:
				doMouseMotion(&event.motion);
				break;

			case SDL_MOUSEWHEEL:
				doMouseWheel(&event.wheel);
				break;

			case SDL_MOUSEBUTTONDOWN:
				doMouseDown(&event.button);
				break;

			case SDL_MOUSEBUTTONUP:
				doMouseUp(&event.button);
				break;

			case SDL_KEYDOWN:
				doKeyDown(&event.key);
				break;

			case SDL_KEYUP:
				doKeyUp(&event.key);
				break;

			case SDL_CONTROLLERAXISMOTION:
				doControllerAxis(&event.caxis);
				break;

			case SDL_CONTROLLERBUTTONDOWN:
				doControllerButtonDown(&event.cbutton);
				break;

			case SDL_CONTROLLERBUTTONUP:
				doControllerButtonUp(&event.cbutton);
				break;

			case SDL_QUIT:
				exit(0);
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						musicSetPlaying(1);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						musicSetPlaying(0);
						break;
				}
				break;
		}
	}

	SDL_GetMouseState(&x, &y);

	app.mouse.x = x;
	app.mouse.y = y;

	if (app.controllerX == CONTROLLER_NOINPUT)
	{
		app.uiMouse.x = x - app.uiOffset.x;
		app.uiMouse.y = y - app.uiOffset.y;
	}
}

void clearInput(void)
{
	SDL_Event event;

	memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	memset(app.mouse.button, 0, sizeof(int) * MAX_MOUSE_BUTTONS);

	while (SDL_PollEvent(&event))
	{
	}

	setMouseCursor(0);
}
