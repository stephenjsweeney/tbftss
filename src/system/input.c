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

#include "input.h"

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
	int x, y;

	SDL_GetMouseState(&x, &y);

	app.mouse.x = x * app.scaleX;
	app.mouse.y = y * app.scaleY;

	blit(mousePointer, app.mouse.x, app.mouse.y, 1);
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
