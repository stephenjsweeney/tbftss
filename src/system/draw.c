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

#include "../system/input.h"
#include "../system/text.h"
#include "../system/util.h"
#include "draw.h"

extern App	  app;
extern Battle battle;
extern Colors colors;
extern Dev	  dev;

static void initColor(SDL_Color *c, int r, int g, int b);

static PointF backgroundPoint[4];

void initGraphics(void)
{
	backgroundPoint[0].x = -app.winWidth / 2;
	backgroundPoint[0].y = -app.winHeight / 2;

	backgroundPoint[1].x = app.winWidth / 2;
	backgroundPoint[1].y = -app.winHeight / 2;

	backgroundPoint[2].x = -app.winWidth / 2;
	backgroundPoint[2].y = app.winHeight / 2;

	backgroundPoint[3].x = app.winWidth / 2;
	backgroundPoint[3].y = app.winHeight / 2;

	initColor(&colors.red, 255, 0, 0);
	initColor(&colors.orange, 255, 128, 0);
	initColor(&colors.yellow, 255, 255, 0);
	initColor(&colors.green, 0, 255, 0);
	initColor(&colors.blue, 0, 0, 255);
	initColor(&colors.cyan, 0, 255, 255);
	initColor(&colors.purple, 255, 0, 255);
	initColor(&colors.white, 255, 255, 255);
	initColor(&colors.black, 0, 0, 0);
	initColor(&colors.lightGrey, 192, 192, 192);
	initColor(&colors.darkGrey, 128, 128, 128);
}

static void initColor(SDL_Color *c, int r, int g, int b)
{
	memset(c, 0, sizeof(SDL_Color));
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = 255;
}

void prepareScene(void)
{
	SDL_SetRenderTarget(app.renderer, app.uiBuffer);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderClear(app.renderer);

	SDL_SetRenderTarget(app.renderer, app.backBuffer);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderClear(app.renderer);
}

void presentScene(void)
{
	SDL_Rect uiDest;

	uiDest.w = UI_WIDTH;
	uiDest.h = UI_HEIGHT;
	uiDest.x = app.uiOffset.x;
	uiDest.y = app.uiOffset.y;

	if (dev.debug)
	{
		drawText(5, app.winHeight - 25, 14, TA_LEFT, colors.white, "DEBUG MODE");

		if (dev.showFPS)
		{
			drawText(app.winWidth / 2, app.winHeight - 25, 14, TA_CENTER, colors.white, "FPS: %d", dev.fps);
		}
	}

	SDL_SetRenderTarget(app.renderer, NULL);
	SDL_RenderCopy(app.renderer, app.backBuffer, NULL, NULL);
	SDL_RenderCopy(app.renderer, app.uiBuffer, NULL, &uiDest);
	if (!app.hideMouse)
	{
		drawMouse();
	}
	SDL_RenderPresent(app.renderer);
}

void blit(AtlasImage *atlasImage, int x, int y, int center)
{
	SDL_Rect dstRect;

	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = atlasImage->rect.w;
	dstRect.h = atlasImage->rect.h;

	if (center)
	{
		dstRect.x -= (dstRect.w / 2);
		dstRect.y -= (dstRect.h / 2);
	}

	SDL_RenderCopy(app.renderer, atlasImage->texture, &atlasImage->rect, &dstRect);
}

void blitScaled(AtlasImage *atlasImage, int x, int y, int w, int h, int center)
{
	SDL_Rect dstRect;

	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = w;
	dstRect.h = h;

	if (center)
	{
		dstRect.x -= (dstRect.w / 2);
		dstRect.y -= (dstRect.h / 2);
	}

	SDL_RenderCopy(app.renderer, atlasImage->texture, &atlasImage->rect, &dstRect);
}

void blitRotated(AtlasImage *atlasImage, int x, int y, float angle)
{
	SDL_Rect dstRect;

	dstRect.x = x;
	dstRect.y = y;
	dstRect.w = atlasImage->rect.w;
	dstRect.h = atlasImage->rect.h;

	dstRect.x -= (dstRect.w / 2);
	dstRect.y -= (dstRect.h / 2);

	SDL_RenderCopyEx(app.renderer, atlasImage->texture, &atlasImage->rect, &dstRect, angle, NULL, SDL_FLIP_NONE);
}

void drawCircle(int cx, int cy, int radius, int r, int g, int b, int a)
{
	int		  x = radius;
	int		  y = 0;
	int		  radiusError = 1 - x;
	SDL_Point p[8];

	SDL_SetRenderDrawColor(app.renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

	while (x >= y)
	{
		p[0].x = x + cx;
		p[0].y = y + cy;
		p[1].x = y + cx;
		p[1].y = x + cy;
		p[2].x = -x + cx;
		p[2].y = y + cy;
		p[3].x = -y + cx;
		p[3].y = x + cy;
		p[4].x = -x + cx;
		p[4].y = -y + cy;
		p[5].x = -y + cx;
		p[5].y = -x + cy;
		p[6].x = x + cx;
		p[6].y = -y + cy;
		p[7].x = y + cx;
		p[7].y = -x + cy;
		SDL_RenderDrawPoints(app.renderer, p, 8);

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

void scrollBackground(float x, float y)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		backgroundPoint[i].x += x;
		backgroundPoint[i].y += y;

		if (backgroundPoint[i].x < 0)
		{
			backgroundPoint[i].x += (app.winWidth * 2);
		}

		if (backgroundPoint[i].x >= app.winWidth)
		{
			backgroundPoint[i].x -= (app.winWidth * 2);
		}

		if (backgroundPoint[i].y < 0)
		{
			backgroundPoint[i].y += (app.winHeight * 2);
		}

		if (backgroundPoint[i].y >= app.winHeight)
		{
			backgroundPoint[i].y -= (app.winHeight * 2);
		}
	}
}

void drawBackground(SDL_Texture *texture)
{
	int		 i;
	SDL_Rect dstRect;

	for (i = 0; i < 4; i++)
	{
		dstRect.x = backgroundPoint[i].x;
		dstRect.y = backgroundPoint[i].y;
		dstRect.w = app.winWidth;
		dstRect.h = app.winHeight;

		SDL_RenderCopy(app.renderer, texture, NULL, &dstRect);
	}
}

int isOnBattleScreen(int x, int y, int w, int h)
{
	x -= (w / 2);
	x -= (app.winWidth / 2);
	x -= battle.camera.x;

	y -= (h / 2);
	y -= (app.winHeight / 2);
	y -= battle.camera.y;

	w *= 2;
	w += app.winWidth;

	h *= 2;
	h += app.winHeight;

	return collision(x, y, w, h, 0, 0, app.winWidth, app.winHeight);
}

void saveScreenshot(void)
{
	static int	 i = 0;
	char		 filename[MAX_NAME_LENGTH];
	SDL_Surface *sshot;

	sprintf(filename, "/tmp/tbftss/%d.bmp", ++i);

	sshot = SDL_CreateRGBSurface(0, app.winWidth, app.winHeight, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(app.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, filename);
	SDL_FreeSurface(sshot);

	if (!dev.takeScreenshots)
	{
		printf("Saved '%s'\n", filename);
	}
}
