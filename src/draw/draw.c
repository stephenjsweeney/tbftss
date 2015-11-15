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

#include "draw.h"

static PointF backgroundPoint[4];

void initBackground(void)
{
	backgroundPoint[0].x = -SCREEN_WIDTH / 2;
	backgroundPoint[0].y = -SCREEN_HEIGHT / 2;
	
	backgroundPoint[1].x = SCREEN_WIDTH / 2;
	backgroundPoint[1].y = -SCREEN_HEIGHT / 2;
	
	backgroundPoint[2].x = -SCREEN_WIDTH / 2;
	backgroundPoint[2].y = SCREEN_HEIGHT / 2;
	
	backgroundPoint[3].x = SCREEN_WIDTH / 2;
	backgroundPoint[3].y = SCREEN_HEIGHT / 2;
}

void prepareScene(void)
{
	SDL_SetRenderTarget(app.renderer, app.backBuffer);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(app.renderer);
}

void presentScene(void)
{
	#if DEBUG
	drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 25, 14, TA_CENTER, colors.white, "FPS: %d", app.fps);
	#endif
	
	SDL_SetRenderTarget(app.renderer, NULL);
	SDL_RenderCopy(app.renderer, app.backBuffer, NULL, NULL);
	SDL_RenderPresent(app.renderer);
}

void blit(SDL_Texture *texture, int x, int y, int center)
{
	SDL_Rect dstRect;
	
	dstRect.x = x;
	dstRect.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &dstRect.w, &dstRect.h);

	if (center)
	{
		dstRect.x -= (dstRect.w / 2);
		dstRect.y -= (dstRect.h / 2);
	}

	SDL_RenderCopy(app.renderer, texture, NULL, &dstRect);
}

void blitScaled(SDL_Texture *texture, int x, int y, int w, int h)
{
	SDL_Rect dstRect;
	
	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = w;
	dstRect.h = h;

	SDL_RenderCopy(app.renderer, texture, NULL, &dstRect);
}

void blitRotated(SDL_Texture *texture, int x, int y, int angle)
{
	SDL_Rect dstRect;
	
	dstRect.x = x;
	dstRect.y = y;
	SDL_QueryTexture(texture, NULL, NULL, &dstRect.w, &dstRect.h);
	dstRect.x -= (dstRect.w / 2);
	dstRect.y -= (dstRect.h / 2);

	SDL_RenderCopyEx(app.renderer, texture, NULL, &dstRect, angle, NULL, SDL_FLIP_NONE);
}

void drawCircle(int cx, int cy, int radius, int r, int g, int b, int a)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;

	SDL_SetRenderDrawColor(app.renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

	while (x >= y)
	{
		SDL_RenderDrawPoint(app.renderer, x + cx,  y + cy);
		SDL_RenderDrawPoint(app.renderer, y + cx,  x + cy);
		SDL_RenderDrawPoint(app.renderer,-x + cx,  y + cy);
		SDL_RenderDrawPoint(app.renderer,-y + cx,  x + cy);
		SDL_RenderDrawPoint(app.renderer,-x + cx, -y + cy);
		SDL_RenderDrawPoint(app.renderer,-y + cx, -x + cy);
		SDL_RenderDrawPoint(app.renderer, x + cx, -y + cy);
		SDL_RenderDrawPoint(app.renderer, y + cx, -x + cy);

		y++;

		if (radiusError < 0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x) + 1;
		}
	}

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void drawFilledCircle(int cx, int cy, int radius, int r, int g, int b, int a)
{
	int x, y;

	SDL_SetRenderDrawColor(app.renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

	for (y = -radius ; y <= radius ; y++)
	{
		for (x =- radius; x <= radius ; x++)
		{
			if (x * x + y * y <= radius * radius)
			{
				SDL_RenderDrawPoint(app.renderer, cx + x, cy + y);
			}
		}
	}

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
}

void scrollBackground(float x, float y)
{
	int i;
	
	for (i = 0 ; i < 4 ; i++)
	{
		backgroundPoint[i].x += x;
		backgroundPoint[i].y += y;
		
		if (backgroundPoint[i].x < 0)
		{
			backgroundPoint[i].x += (SCREEN_WIDTH * 2);
		}
		
		if (backgroundPoint[i].x >= SCREEN_WIDTH)
		{
			backgroundPoint[i].x -= (SCREEN_WIDTH * 2);
		}
		
		if (backgroundPoint[i].y < 0)
		{
			backgroundPoint[i].y += (SCREEN_HEIGHT * 2);
		}
		
		if (backgroundPoint[i].y >= SCREEN_HEIGHT)
		{
			backgroundPoint[i].y -= (SCREEN_HEIGHT * 2);
		}
	}
}

void drawBackground(SDL_Texture *texture)
{
	int i;
	
	for (i = 0 ; i < 4 ; i++)
	{
		blitScaled(texture, backgroundPoint[i].x, backgroundPoint[i].y, SCREEN_WIDTH, SCREEN_HEIGHT);
	}
}

void saveScreenshot(void)
{
	static int i = 0;
	char filename[MAX_NAME_LENGTH];
	SDL_Surface *sshot;
	
	sprintf(filename, "/tmp/tbftss/%d.bmp", ++i);
	
	sshot = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(app.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, filename);
	SDL_FreeSurface(sshot);
}
