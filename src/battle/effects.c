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

#include "effects.h"

static void setRandomFlameHue(Effect *e);
static void setRandomShieldHue(Effect *e);
static void resizeDrawList(void);
static int pointOnScreen(float x, float y);

static SDL_Texture *explosionTexture;
static SDL_Texture *shieldHitTexture;
static SDL_Texture *haloTexture;
static Effect **effectsToDraw;
static int drawCapacity;

void initEffects(void)
{
	explosionTexture = getTexture("gfx/effects/explosion.png");
	shieldHitTexture = getTexture("gfx/effects/shieldHit.png");
	haloTexture = getTexture("gfx/effects/halo.png");

	drawCapacity = INITIAL_EFFECT_DRAW_CAPACITY;

	effectsToDraw = malloc(sizeof(Effect*) * drawCapacity);
	memset(effectsToDraw, 0, sizeof(Effect*) * drawCapacity);
}

void doEffects(void)
{
	int i, onScreen;
	Effect *e;
	Effect *prev = &battle.effectHead;
	
	i = 0;
	
	memset(effectsToDraw, 0, sizeof(Effect*) * drawCapacity);

	for (e = battle.effectHead.next ; e != NULL ; e = e->next)
	{
		e->x += e->dx;
		e->y += e->dy;

		e->a -= (e->type != EFFECT_ECM) ? 1 : 3;
		e->a = MAX(0, e->a);

		e->health--;

		e->size += e->scaleAmount;

		if (e->health <= 0)
		{
			if (e == battle.effectTail)
			{
				battle.effectTail = prev;
			}

			prev->next = e->next;
			free(e);
			e = prev;
		}
		else
		{
			onScreen = 0;
			
			switch (e->type)
			{
				case EFFECT_LINE:
					onScreen = pointOnScreen(e->x - battle.camera.x, e->y - battle.camera.y) || pointOnScreen(e->x + (e->dx * 3) - battle.camera.x, e->y + (e->dy * 3) - battle.camera.y);
					break;
					
				case EFFECT_POINT:
					onScreen = pointOnScreen(e->x - battle.camera.x, e->y - battle.camera.y);
					break;
					
				default:
					onScreen = isOnBattleScreen(e->x, e->y, e->size, e->size);
					break;
			}
				
			if (onScreen)
			{
				effectsToDraw[i++] = e;

				if (i == drawCapacity)
				{
					resizeDrawList();
				}
			}
		}

		prev = e;
	}
}

static int pointOnScreen(float x, float y)
{
	return collision(x, y, 1, 1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

static void resizeDrawList(void)
{
	int n;

	n = drawCapacity + INITIAL_EFFECT_DRAW_CAPACITY;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Resizing effect draw capacity: %d -> %d", drawCapacity, n);

	effectsToDraw = resize(effectsToDraw, sizeof(Effect*) * drawCapacity, sizeof(Effect*) * n);
	
	drawCapacity = n;
}

void drawEffects(void)
{
	int i;
	Effect *e;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

	for (i = 0, e = effectsToDraw[i] ; e != NULL ; e = effectsToDraw[++i])
	{
		SDL_SetRenderDrawColor(app.renderer, e->r, e->g, e->b, e->a);

		SDL_SetTextureBlendMode(e->texture, SDL_BLENDMODE_ADD);
		SDL_SetTextureAlphaMod(e->texture, e->a);

		switch (e->type)
		{
			case EFFECT_POINT:
				SDL_RenderDrawPoint(app.renderer, e->x - battle.camera.x, e->y - battle.camera.y);
				break;

			case EFFECT_LINE:
				SDL_RenderDrawLine(app.renderer, e->x - battle.camera.x, e->y - battle.camera.y, e->x + (e->dx * 3) - battle.camera.x, e->y + (e->dy * 3) - battle.camera.y);
				break;

			case EFFECT_TEXTURE:
				SDL_SetTextureColorMod(e->texture, e->r, e->g, e->b);
				blitScaled(e->texture, e->x - battle.camera.x, e->y - battle.camera.y, e->size, e->size, 0);
				break;

			case EFFECT_HALO:
				SDL_SetTextureColorMod(e->texture, e->r, e->g, e->b);
				blitScaled(e->texture, e->x - battle.camera.x - (e->size / 2), e->y - battle.camera.y - (e->size / 2), e->size, e->size, 0);
				break;

			case EFFECT_ECM:
				SDL_SetTextureColorMod(e->texture, e->r, e->g, e->b);
				blitScaled(e->texture, SCREEN_WIDTH / 2 - (e->size / 2), SCREEN_HEIGHT / 2 - (e->size / 2), e->size, e->size, 0);
				break;
		}
	}

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void drawShieldHitEffect(Entity *e)
{
	int size = MAX(e->w, e->h) + 32;
	
	SDL_SetTextureBlendMode(shieldHitTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(shieldHitTexture, e->shieldHit);
	blitScaled(shieldHitTexture, e->x - battle.camera.x, e->y - battle.camera.y, size, size, 1);
}

void addBulletHitEffect(int x, int y, int r, int g, int b)
{
	Effect *e;
	int i;
	
	for (i = 0 ; i < 4 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_TEXTURE;
		e->texture = explosionTexture;
		e->size = 16;
		e->x = x;
		e->y = y;
		
		e->dx = (rand() % 25) - (rand() % 25);
		e->dx *= 0.01;
		e->dy = (rand() % 25) - (rand() % 25);
		e->dy *= 0.01;

		e->r = r;
		e->g = g;
		e->b = b;
		e->a = 64;
		e->health = e->a;
	}
}

void addSmallFighterExplosion(void)
{
	Effect *e;

	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;

	e->type = EFFECT_TEXTURE;

	e->x = self->x + (rand() % 16 - rand() % 16);
	e->y = self->y + (rand() % 16 - rand() % 16);
	e->texture = explosionTexture;
	e->size = 32;

	setRandomFlameHue(e);

	e->a = 128 + (rand() % 128);
	e->health = e->a;

	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addDebrisFire(int x, int y)
{
	Effect *e;

	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;

	e->type = EFFECT_TEXTURE;

	e->x = x + (rand() % 8 - rand() % 8);
	e->y = y + (rand() % 8 - rand() % 8);
	e->texture = explosionTexture;
	e->size = 4 + rand() % 12;

	setRandomFlameHue(e);

	e->a = rand() % 256;
	e->health = e->a;

	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addSmallExplosion(void)
{
	int i;
	Effect *e;

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
		e->texture = explosionTexture;
		e->size = 32 + (rand() % 64);
		e->r = 255;

		setRandomFlameHue(e);

		e->a = 128 + (rand() % 128);
		e->health = e->a;

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

		e->a = 128;
		e->health = e->a;

		setRandomFlameHue(e);
	}
}

void addMineExplosion(void)
{
	int i;
	Effect *e;

	for (i = 0 ; i < 64 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_TEXTURE;

		e->x = self->x + rand() % 64 - rand() % 64;
		e->y = self->y + rand() % 64 - rand() % 64;
		e->texture = explosionTexture;
		e->size = 64 + (rand() % 64);
		e->r = 255;

		setRandomFlameHue(e);

		e->a = 32 + (rand() % 192);
		e->health = e->a;

		e->x -= e->size / 2;
		e->y -= e->size / 2;
	}

	e = malloc(sizeof(Effect));

	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;

	e->type = EFFECT_HALO;
	e->x = self->x;
	e->y = self->y;
	e->size = 64;
	e->scaleAmount = 6;
	e->texture = haloTexture;

	e->r = 255;
	e->g = 255;
	e->b = 255;
	e->a = 128;

	e->health = 128;
}

void addLargeExplosion(void)
{
	int i;
	Effect *e;

	for (i = 0 ; i < 64 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_TEXTURE;

		e->x = self->x + rand() % 255 - rand() % 255;
		e->y = self->y + rand() % 255 - rand() % 255;
		e->dx = (rand() % 25) - (rand() % 25);
		e->dx *= 0.01;
		e->dy = (rand() % 25) - (rand() % 25);
		e->dy *= 0.01;
		e->texture = explosionTexture;
		e->size = 128 + (rand() % 512);
		e->r = 255;

		setRandomFlameHue(e);

		e->a = 128 + (rand() % 128);
		e->health = e->a;

		e->x -= e->size / 2;
		e->y -= e->size / 2;
	}
	
	e = malloc(sizeof(Effect));

	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;

	e->type = EFFECT_HALO;
	e->x = self->x;
	e->y = self->y;
	e->size = 256;
	e->scaleAmount = 4;
	e->texture = haloTexture;

	e->r = 255;
	e->g = 255;
	e->b = 255;
	e->a = 255;

	e->health = 255;
}

void addMissileExplosion(Bullet *b)
{
	int i;
	Effect *e;

	for (i = 0 ; i < 8 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_TEXTURE;

		e->x = b->x;
		e->y = b->y;
		e->dx = (rand() % 25) - (rand() % 25);
		e->dx *= 0.025;
		e->dy = (rand() % 25) - (rand() % 25);
		e->dy *= 0.025;
		e->texture = explosionTexture;
		e->size = 32 + (rand() % 64);
		e->r = 255;

		setRandomFlameHue(e);

		e->a = 128 + (rand() % 128);
		e->health = e->a;

		e->x -= e->size / 2;
		e->y -= e->size / 2;
	}

	for (i = 0 ; i < 24 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_LINE;
		e->x = b->x;
		e->y = b->y;
		e->dx = rand() % 64 - rand() % 64;
		e->dx *= 0.1;
		e->dy = rand() % 64 - rand() % 64;
		e->dy *= 0.1;

		e->a = 128;
		e->health = e->a;

		setRandomFlameHue(e);
	}
}

void addEngineEffect(void)
{
	Effect *e;
	float c, s;
	int h;

	e = malloc(sizeof(Effect));
	memset(e, 0, sizeof(Effect));
	battle.effectTail->next = e;
	battle.effectTail = e;

	e->type = EFFECT_TEXTURE;

	s = sin(TO_RAIDANS(self->angle));
	c = cos(TO_RAIDANS(self->angle));

	h = self->h / 2;

	e->x = -(h * s) + self->x;
	e->y = (h * c) + self->y;

	e->x += rand() % 4 - rand() % 4;

	e->texture = explosionTexture;
	e->size = 16;
	e->r = 128;
	e->g = 128;
	e->b = 255;
	e->a = 64;

	e->health = e->a;

	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addLargeEngineEffect(void)
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

	e->texture = explosionTexture;
	e->size = 64;
	e->r = 128;
	e->g = 128;
	e->b = 255;
	e->a = 64;

	e->health = e->a;

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

	e->texture = explosionTexture;
	e->size = 12;
	setRandomFlameHue(e);
	e->a = 128;

	e->health = e->a;

	e->x -= e->size / 2;
	e->y -= e->size / 2;
}

void addShieldSplinterEffect(Entity *ent)
{
	int i;
	Effect *e;

	for (i = 0 ; i < 48 ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_LINE;
		e->x = ent->x;
		e->y = ent->y;
		e->dx = rand() % 64 - rand() % 64;
		e->dx *= 0.1;
		e->dy = rand() % 64 - rand() % 64;
		e->dy *= 0.1;
		e->a = 255;
		
		e->health = e->a;

		setRandomShieldHue(e);
	}
}

void addECMEffect(Entity *ent)
{
	int i;
	Effect *e;

	for (i = 0 ; i < 3 ; i++)
	{
		e = malloc(sizeof(Effect));

		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		e->type = EFFECT_ECM;
		e->x = ent->x;
		e->y = ent->y;
		e->size = i * 4;
		e->scaleAmount = 5;
		e->texture = haloTexture;

		e->r = 128;
		e->g = 128 + (i * 64);
		e->b = 255;
		e->a = 255;

		e->health = 255;
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

static void setRandomShieldHue(Effect *e)
{
	e->b = 255;

	switch (rand() % 4)
	{
		case 0:
			e->g = 128;
			break;

		case 1:
			e->g = 196;
			break;

		case 2:
			e->g = 255;
			break;

		case 3:
			e->r = e->g = 255;
			break;
	}
}

void destroyEffects(void)
{
	if (effectsToDraw)
	{
		free(effectsToDraw);
	}

	effectsToDraw = NULL;
}
