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

#include "input.h"

static SDL_Texture *mousePointer;

void initInput(void)
{
	memset(&app.mouse, 0, sizeof(Mouse));
	
	mousePointer = getTexture("gfx/input/mousePointer.png");
	
	SDL_QueryTexture(mousePointer, NULL, NULL, &app.mouse.w, &app.mouse.h);
}

void doMouseDown(SDL_MouseButtonEvent *event)
{
	app.mouse.button[event->button] = 1;
}

void doMouseUp(SDL_MouseButtonEvent *event)
{
	app.mouse.button[event->button] = 0;
}

void doMouseWheel(SDL_MouseWheelEvent *event)
{
	if (event->y == -1)
	{
		app.mouse.button[SDL_BUTTON_X1] = 1;
	}
	
	if (event->y == 1)
	{
		app.mouse.button[SDL_BUTTON_X2] = 1;
	}
}

void doMouseMotion(SDL_MouseMotionEvent *event)
{
	app.mouse.dx = event->xrel;
	app.mouse.dy = event->yrel;
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
	int i;
	
	for (i = 0 ; i < MAX_KEYBOARD_KEYS ; i++)
	{
		app.keyboard[i] = 0;
	}
	
	for (i = 0 ; i < MAX_MOUSE_BUTTONS ; i++)
	{
		app.mouse.button[i] = 0;
	}
	
	while (SDL_PollEvent(&event))
	{
	}
}
