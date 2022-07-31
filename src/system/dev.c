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

#include "../galaxy/mission.h"
#include "dev.h"

extern App	  app;
extern Battle battle;
extern Dev	  dev;

void doDevKeys(void)
{
	if (dev.debug)
	{
		if (app.keyboard[SDL_SCANCODE_1])
		{
			dev.playerImmortal = !dev.playerImmortal;
			app.keyboard[SDL_SCANCODE_1] = 0;
			printf("DEBUG: dev.playerImmortal=%d\n", dev.playerImmortal);
		}

		if (app.keyboard[SDL_SCANCODE_2])
		{
			dev.playerUnlimitedMissiles = !dev.playerUnlimitedMissiles;
			app.keyboard[SDL_SCANCODE_2] = 0;
			printf("DEBUG: dev.playerUnlimitedMissiles=%d\n", dev.playerUnlimitedMissiles);
		}

		if (app.keyboard[SDL_SCANCODE_3])
		{
			dev.noAIWeapons = !dev.noAIWeapons;
			app.keyboard[SDL_SCANCODE_3] = 0;
			printf("DEBUG: dev.noAIWeapons=%d\n", dev.noAIWeapons);
		}

		if (app.keyboard[SDL_SCANCODE_4])
		{
			dev.noEntityActions = !dev.noEntityActions;
			app.keyboard[SDL_SCANCODE_4] = 0;
			printf("DEBUG: dev.noEntityActions=%d\n", dev.noEntityActions);
		}

		if (app.keyboard[SDL_SCANCODE_5])
		{
			dev.allImmortal = !dev.allImmortal;
			app.keyboard[SDL_SCANCODE_5] = 0;
			printf("DEBUG: dev.allImmortal=%d\n", dev.allImmortal);
		}

		if (app.keyboard[SDL_SCANCODE_6])
		{
			completeMission();
			battle.missionFinishedTimer = -FPS;
		}

		if (app.keyboard[SDL_SCANCODE_9])
		{
			dev.showFPS = !dev.showFPS;
			app.keyboard[SDL_SCANCODE_9] = 0;
			printf("DEBUG: dev.showFPS=%d\n", dev.showFPS);
		}

		if (app.keyboard[SDL_SCANCODE_0])
		{
			dev.takeScreenshots = !dev.takeScreenshots;
			app.keyboard[SDL_SCANCODE_0] = 0;
			printf("DEBUG: dev.takeScreenshots=%d\n", dev.takeScreenshots);
		}
	}
}
