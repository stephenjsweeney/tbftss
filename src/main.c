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
static void capFrameRate(long *then, float *remainder);

int main(int argc, char *argv[])
{
	long then, lastFrameTime, frames;
	float remainder;
	
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
	
	remainder = 0;
	dev.fps = frames = 0;
	then = SDL_GetTicks();
	lastFrameTime = SDL_GetTicks() + 1000;
	
	while (1)
	{
		capFrameRate(&then, &remainder);
		
		doInput();
		
		if (app.modalDialog.type != MD_NONE)
		{
			doModalDialog();
		}
		
		/* let the delegate decide during logic() */
		app.doTrophyAlerts = 0;
		
		app.delegate.logic();
		
		if (app.doTrophyAlerts)
		{
			doTrophyAlerts();
		}
		
		game.stats[STAT_TIME]++;
		
		/* always zero the mouse motion */
		app.mouse.dx = app.mouse.dy = 0;
		
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

static void capFrameRate(long *then, float *remainder)
{
	long wait;
	
	wait = 16 + *remainder;
	
	*remainder -= (int)*remainder;
	
	wait -= (SDL_GetTicks() - *then);
	
	if (wait < 1)
	{
		wait = 1;
	}
	
	SDL_Delay(wait);
	
	*remainder += 0.667;
	
	*then = SDL_GetTicks();
}

static void handleLoggingArgs(int argc, char *argv[])
{
	int i;
	
	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN);
	
	for (i = 1 ; i < argc ; i++)
	{
		if (strcmp(argv[i], "-debug") == 0)
		{
			dev.debug = 1;
			
			SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
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
		if (strcmp(argv[i], "-mission") == 0)
		{
			loadTestMission(argv[++i]);
			
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
