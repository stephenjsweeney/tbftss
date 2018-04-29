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

#include "main.h"

static void handleMissionArgs(int argc, char *argv[]);
static void handleLoggingArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	float td;
	long then, lastFrameTime, frames;
	long expireTextTimer;
	SDL_Event event;
	
	memset(&app, 0, sizeof(App));
	memset(&dev, 0, sizeof(Dev));
	
	handleLoggingArgs(argc, argv);
	
	atexit(cleanup);

	srand(time(NULL));
	
	init18N(argc, argv);
	
	initLookups();

	initSDL();
	
	initGameSystem();
	
	createScreenshotFolder();
	
	if (fileExists(getSaveFilePath(SAVE_FILENAME)))
	{
		loadGame();
	}
	
	handleMissionArgs(argc, argv);
	
	dev.fps = frames = td = 0;
	then = SDL_GetTicks();
	lastFrameTime = SDL_GetTicks() + 1000;
	expireTextTimer = SDL_GetTicks() + (1000 * 10);
	
	while (1)
	{
		td += (SDL_GetTicks() - then);
		
		then = SDL_GetTicks();
		
		while (td >= LOGIC_RATE)
		{
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
			
			if (app.modalDialog.type != MD_NONE)
			{
				doModalDialog();
			}
			
			/* let the delegate decide during logic() */
			app.doTrophyAlerts = 0;
			
			app.delegate.logic();
			
			td -= LOGIC_RATE;
			
			if (app.doTrophyAlerts)
			{
				doTrophyAlerts();
			}
			
			if (app.resetTimeDelta)
			{
				td = 0;
				then = SDL_GetTicks();
				app.resetTimeDelta = 0;
			}
			
			game.stats[STAT_TIME]++;
			
			/* always zero the mouse motion */
			app.mouse.dx = app.mouse.dy = 0;
		}
		
		prepareScene();

		app.delegate.draw();
		
		if (app.doTrophyAlerts)
		{
			drawTrophyAlert();
		}
		
		if (app.modalDialog.type != MD_NONE)
		{
			drawModalDialog();
		}
		
		presentScene();
		
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
		
		if (isControl(CONTROL_SCREENSHOT))
		{
			saveScreenshot();
			
			clearControl(CONTROL_SCREENSHOT);
		}
		
		if (SDL_GetTicks() > expireTextTimer)
		{
			expireTexts(0);
			
			expireTextTimer = SDL_GetTicks() + (1000 * 10);
		}
		
		/* don't save more than once per request, and not in the middle of battle */
		if (app.saveGame && battle.status != MS_IN_PROGRESS)
		{
			saveGame();
			app.saveGame = 0;
		}

		SDL_Delay(1);
	}

	return 0;
}

static void handleLoggingArgs(int argc, char *argv[])
{
	int i;
	
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
	
	for (i = 1 ; i < argc ; i++)
	{
		if (strcmp(argv[i], "-debug") == 0)
		{
			dev.debug = 1;
			
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
		}
		
		if (strcmp(argv[i], "-warn") == 0)
		{
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN);
		}
		
		if (strcmp(argv[i], "-info") == 0)
		{
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
		}
	}
}

static void handleMissionArgs(int argc, char *argv[])
{
	int i, testingMission, showCredits;
	
	showCredits = testingMission = 0;
	
	for (i = 1 ; i < argc ; i++)
	{
		/* assume this is filename for testing */
		if (argv[i][0] != '-')
		{
			loadTestMission(argv[i]);
			
			testingMission = 1;
		}
		
		if (strcmp(argv[i], "-credits") == 0)
		{
			showCredits = 1;
		}
	}
	
	if (showCredits)
	{
		initCredits();
	}
	else if (!testingMission)
	{
		initTitle();
	}
}
