/*
Copyright (C) 2015 Parallel Realities

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

#include "main.h"

static void handleArguments(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	float td;
	long then, lastFrameTime, frames;
	long expireTextTimer;
	
	SDL_Event event;
	
	memset(&app, 0, sizeof(App));
	memset(&dev, 0, sizeof(Dev));
	
	atexit(cleanup);

	srand(time(NULL));
	
	setlocale(LC_NUMERIC, "");

	initSDL();
	
	initGameSystem();
	
	handleArguments(argc, argv);
	
	dev.fps = frames = td = 0;
	then = SDL_GetTicks();
	lastFrameTime = SDL_GetTicks() + 1000;
	expireTextTimer = SDL_GetTicks() + (1000 * 10);
	
	while (1)
	{
		td += (SDL_GetTicks() - then);
		
		then = SDL_GetTicks();
		
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
					if (event.key.keysym.scancode >= 0 && event.key.keysym.scancode < MAX_KEYBOARD_KEYS && event.key.repeat == 0)
					{
						app.keyboard[event.key.keysym.scancode] = 1;
					}
					break;
					
				case SDL_KEYUP:
					if (event.key.keysym.scancode >= 0 && event.key.keysym.scancode < MAX_KEYBOARD_KEYS)
					{
						app.keyboard[event.key.keysym.scancode] = 0;
					}
					break;

				case SDL_QUIT:
					exit(0);
					break;
			}
		}
		
		while (td >= LOGIC_RATE)
		{
			app.delegate.logic();
			td -= LOGIC_RATE;
			
			game.stats[STAT_TIME]++;
		}

		app.delegate.draw();
		
		doDevKeys();
		
		frames++;
		
		if (SDL_GetTicks() > lastFrameTime)
		{
			dev.fps = frames;
			frames = 0;
			lastFrameTime = SDL_GetTicks() + 1000;
			
			if (dev.takeScreenshots)
			{
				saveScreenshot();
			}
		}
		
		if (SDL_GetTicks() > expireTextTimer)
		{
			expireTexts(0);
			
			expireTextTimer = SDL_GetTicks() + (1000 * 10);
		}
		
		/* always zero the mouse motion */
		app.mouse.dx = app.mouse.dy = 0;

		SDL_Delay(1);
	}

	return 0;
}

static void handleArguments(int argc, char *argv[])
{
	int i;
	int testingMission = 0;
	
	for (i = 1 ; i < argc ; i++)
	{
		/* assume this is filename for testing */
		if (argv[i][0] != '-')
		{
			loadTestMission(argv[i]);
			
			testingMission = 1;
		}
		else
		{
			if (strcmp(argv[i], "-debug") == 0)
			{
				dev.debug = 1;
			}
		}
	}
	
	if (!testingMission)
	{
		initTitle();
	}
}
