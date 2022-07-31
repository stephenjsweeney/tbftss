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

#include "../system/draw.h"
#include "../system/input.h"
#include "transition.h"

static long transitionStartTime;

void startSectionTransition(void)
{
	transitionStartTime = SDL_GetTicks();

	prepareScene();

	clearInput();

	presentScene();
}

void endSectionTransition(void)
{
	long elapsed = 250 - (SDL_GetTicks() - transitionStartTime);

	if (elapsed > 0)
	{
		SDL_Delay(elapsed);
	}
}
