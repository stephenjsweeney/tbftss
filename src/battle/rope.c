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

#include "rope.h"

void attachRope(void)
{
	int i, distance;
	Entity *e, **candidates;
	
	if ((self->flags & EF_HAS_ROPE) && self->towing == NULL)
	{
		candidates = getAllEntsWithin(self->x - (self->w / 2), self->y - (self->h / 2), self->w, self->h, self);
	
		for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
		{
			if (!e->owner && e->type == ET_FIGHTER && (e->flags & EF_DISABLED) && (e->flags & EF_ROPED_ATTACHED) == 0 && e->alive == ALIVE_ALIVE)
			{
				distance = getDistance(e->x, e->y, self->x, self->y);
				
				if (distance > 0 && distance <= self->separationRadius)
				{
					self->towing = e;
					self->aiFlags |= AIF_GOAL_JUMPGATE;
					
					e->owner = self;
					e->speed = 1;
					e->flags |= EF_RETREATING;
					e->flags |= EF_ROPED_ATTACHED;
					
					runScriptFunction("TOWING %s", e->name);
					
					if (self == player)
					{
						battle.stats[STAT_NUM_TOWED]++;
						addHudMessage(colors.white, _("Tow rope attached"));
					}
				}
			}
		}
	}
}

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

void drawRope(Entity *e)
{
	if (e->towing)
	{
		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
	
		SDL_RenderDrawLine(app.renderer, e->x - battle.camera.x, e->y - battle.camera.y, e->towing->x - battle.camera.x, e->towing->y - battle.camera.y);
	}
}

void cutRope(Entity *e)
{
	/* thing being towed is dead */
	if (e->owner && e->owner->towing == e)
	{
		e->owner->towing = NULL;
		e->owner->aiFlags &= ~AIF_GOAL_JUMPGATE;
	}
	
	/* tug is dead - reset thing being tugged */
	if (e->towing)
	{
		e->towing->flags &= ~EF_RETREATING;
		e->towing->flags &= ~EF_ROPED_ATTACHED;
		e->towing->speed = 0;
		e->towing = NULL;
	}
}
