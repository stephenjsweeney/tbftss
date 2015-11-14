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

#include "rope.h"

void doRope(Entity *owner)
{
	float dx, dy, angle, force;
	int distance;
	
	if (owner->towing)
	{
		distance = getDistance(owner->towing->x, owner->towing->y, owner->x, owner->y);
		
		if (distance > ROPE_DISTANCE)
		{
			angle = getAngle(owner->x, owner->y, owner->towing->x, owner->towing->y);
				
			dx = sin(TO_RAIDANS(angle));
			dy = -cos(TO_RAIDANS(angle));
			force = (distance - ROPE_DISTANCE) * 0.02;
			
			owner->towing->dx -= (dx * force);
			owner->towing->dy -= (dy * force);
		}
		
		owner->towing->dx *= 0.985;
		owner->towing->dy *= 0.985;
	}
}

void drawRope(Entity *owner)
{
	if (owner->towing)
	{
		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
	
		SDL_RenderDrawLine(app.renderer, owner->x - battle.camera.x, owner->y - battle.camera.y, owner->towing->x - battle.camera.x, owner->towing->y - battle.camera.y);
	}
}
