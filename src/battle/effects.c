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

#include "effects.h"

static void setRandomFlameHue(Effect *e);

void doEffects(void)
{
	Effect *e;
	Effect *prev = &battle.effectHead;
	
	for (e = battle.effectHead.next ; e != NULL ; e = e->next)
	{
		e->x += e->dx;
		e->y += e->dy;
		
		e->x -= battle.ssx;
		e->y -= battle.ssy;
		
		e->health--;
		
		if (e->health <= 0)
		{
			if (--e->a <= 0)
			{
				if (e == battle.effectTail)
				{
					battle.effectTail = prev;
				}
				
				prev->next = e->next;
				free(e);
				e = prev;
			}
		}
		
		prev = e;
	}
}

void drawEffects(void)
{
	Effect *e;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	
	for (e = battle.effectHead.next ; e != NULL ; e = e->next)
	{
		SDL_SetRenderDrawColor(app.renderer, e->r, e->g, e->b, e->a);
		
		SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_ADD);
		SDL_SetTextureAlphaMod(e->texture, e->a);
		
		switch (e->type)
		{
			case EFFECT_LINE:
				SDL_RenderDrawLine(app.renderer, e->x, e->y, e->x + (e->dx * 3), e->y + (e->dy * 3));
				break;
				
			case EFFECT_TEXTURE:
				SDL_SetTextureColorMod(e->texture, e->r, e->g, e->b);
				blitScaled(e->texture, e->x, e->y, e->size, e->size);
				break;
				
			case EFFECT_HALO:
				break;
		}
	}
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void addSmallFighterExplosion(void)
{
	Effect *e;
	SDL_Texture *t = getTexture("gfx/battle/explosion.png");
	
	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;
	
	e->type = EFFECT_TEXTURE;
	
	e->x = self->x + (rand() % 16 - rand() % 16);
	e->y = self->y + (rand() % 16 - rand() % 16);
	e->texture = t;
	e->health = 0;
	e->size = 32;
	
	setRandomFlameHue(e);
	
	e->a = 128 + (rand() % 128);
	
	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addFighterExplosion(void)
{
	int i;
	Effect *e;
	SDL_Texture *t = getTexture("gfx/battle/explosion.png");
	
	for (i = 0 ; i < 32 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;
		
		e->type = EFFECT_TEXTURE;
		
		e->x = self->x;
		e->y = self->y;
		e->dx = (rand() % 25) - (rand() % 25);
		e->dx *= 0.025;
		e->dy = (rand() % 25) - (rand() % 25);
		e->dy *= 0.025;
		e->texture = t;
		e->health = 0;
		e->size = 32 + (rand() % 64);
		e->r = 255;
		
		setRandomFlameHue(e);
		
		e->a = 128 + (rand() % 128);
		
		e->x -= e->size / 2;
		e->y -= e->size / 2;
	}
	
	for (i = 0 ; i < 96 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;
		
		e->type = EFFECT_LINE;
		e->x = self->x;
		e->y = self->y;
		e->dx = rand() % 64 - rand() % 64;
		e->dx *= 0.1;
		e->dy = rand() % 64 - rand() % 64;
		e->dy *= 0.1;
		e->health = FPS / 2;
		
		e->a = 128;
		
		setRandomFlameHue(e);
	}
}

static void setRandomFlameHue(Effect *e)
{
	e->r = 255;
	
	switch (rand() % 4)
	{
		case 0:
			break;
			
		case 1:
			e->g = 128;
			break;
			
		case 2:
			e->g = 255;
			break;
			
		case 3:
			e->g = e->b = 255;
			break;
	}
}

void addEngineEffect(void)
{
	Effect *e;
	
	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;
	
	e->type = EFFECT_TEXTURE;
	
	e->x = self->x;
	e->y = self->y;
	
	e->x -= sin(TO_RAIDANS(self->angle)) * 16;
	e->y -= -cos(TO_RAIDANS(self->angle)) * 16;
	
	e->x += rand() % 4;
	e->x -= rand() % 4;
	
	e->texture = getTexture("gfx/battle/explosion.png");
	e->health = 0;
	e->size = 16;
	e->r = 128;
	e->g = 128;
	e->b = 255;
	e->a = 64;
	
	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addMissileEngineEffect(Bullet *b)
{
	Effect *e;
	
	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;
	
	e->type = EFFECT_TEXTURE;
	
	e->x = b->x;
	e->y = b->y;
	
	e->x -= sin(TO_RAIDANS(b->angle)) * 10;
	e->y -= -cos(TO_RAIDANS(b->angle)) * 10;
	
	e->x += rand() % 4;
	e->x -= rand() % 4;
	
	e->texture = getTexture("gfx/battle/explosion.png");
	e->health = 0;
	e->size = 12;
	setRandomFlameHue(e);
	e->a = 128;
	
	e->x -= e->size / 2;
	e->y -= e->size / 2;
}
