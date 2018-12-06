/*
Copyright (C) 2018 Parallel Realities

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

#include "text.h"

static void initFont(char *name, char *filename);
static void drawWord(char *word, int *x, int *y, int startX);
static void applyWordWrap(char *word, int *x, int *y, int startX);
void calcTextDimensions(char *text, int size, int *w, int *h);
void useFont(char *name);

static SDL_Color white = {255, 255, 255, 255};
static char drawTextBuffer[1024];
static Font fontHead;
static Font *fontTail;
static Font *activeFont = NULL;
static float scale;

void initFonts(void)
{
	memset(&fontHead, 0, sizeof(Font));
	fontTail = &fontHead;
	
	initFont("roboto", getFileLocation("data/fonts/Roboto-Medium.ttf"));
	initFont("khosrau", getFileLocation("data/fonts/Khosrau.ttf"));
	
	useFont("roboto");
}

static void initFont(char *name, char *filename)
{
	SDL_Texture *texture;
	TTF_Font *font;
	Font *f;
	SDL_Surface *surface, *text;
	SDL_Rect dest;
	int i;
	char c[2];
		
	surface = SDL_CreateRGBSurface(0, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 32, 0, 0, 0, 0xff);
	
	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
	
	font = TTF_OpenFont(filename, FONT_SIZE);
	
	f = malloc(sizeof(Font));
	memset(f, 0, sizeof(Font));
	
	dest.x = dest.y = 0;
	
	for (i = ' ' ; i <= 'z' ; i++)
	{
		memset(c, 0, 2);
		
		sprintf(c, "%c", i);
		
		text = TTF_RenderUTF8_Blended(font, c, white);
		
		TTF_SizeText(font, c, &dest.w, &dest.h);
		
		if (dest.x + dest.w >= FONT_TEXTURE_SIZE)
		{
			dest.x = 0;
			
			dest.y += dest.h + 1;
		}
		
		SDL_BlitSurface(text, NULL, surface, &dest);
		
		f->glyphs[i].rect = dest;
		
		SDL_FreeSurface(text);
		
		dest.x += dest.w;
	}
	
	TTF_CloseFont(font);
	
	texture = toTexture(surface, 1);
	
	f->texture = texture;
	
	for (i = 0 ; i < 128 ; i++)
	{
		f->glyphs[i].texture = texture;
	}
	
	strcpy(f->name, name);
	
	fontTail->next = f;
	fontTail = f;
}

void drawText(int x, int y, int size, int align, SDL_Color color, const char *format, ...)
{
	int i, startX, n, w, h;
	char word[128];
	va_list args;
	
	if (activeFont)
	{
		SDL_SetTextureColorMod(activeFont->texture, color.r, color.g, color.b);
		SDL_SetTextureAlphaMod(activeFont->texture, color.a);
		
		memset(&drawTextBuffer, '\0', sizeof(drawTextBuffer));

		va_start(args, format);
		vsprintf(drawTextBuffer, format, args);
		va_end(args);
		
		scale = size / (FONT_SIZE * 1.0f);
		
		startX = x;
		
		memset(word, 0, 128);
		
		n = 0;
		
		calcTextDimensions(drawTextBuffer, size, &w, &h);
		
		if (align == TA_RIGHT)
		{
			x -= w;
		}
		else if (align == TA_CENTER)
		{
			x -= (w / 2);
		}
		
		for (i = 0 ; i < strlen(drawTextBuffer) ; i++)
		{
			word[n++] = drawTextBuffer[i];
			
			if (drawTextBuffer[i] == ' ')
			{
				drawWord(word, &x, &y, startX);
				
				memset(word, 0, 128);
				
				n = 0;
			}
		}
		
		drawWord(word, &x, &y, startX);
	}
}

static void drawWord(char *word, int *x, int *y, int startX)
{
	int i, c;
	SDL_Rect dest;
	
	if (app.textWidth > 0)
	{
		applyWordWrap(word, x, y, startX);
	}
	
	for (i = 0 ; i < strlen(word) ; i++)
	{
		c = word[i];
		
		dest.x = *x;
		dest.y = *y;
		dest.w = activeFont->glyphs[c].rect.w * scale;
		dest.h = activeFont->glyphs[c].rect.h * scale;
		
		SDL_RenderCopy(app.renderer, activeFont->texture, &activeFont->glyphs[c].rect, &dest);
		
		*x += activeFont->glyphs[c].rect.w * scale;
	}
}

static void applyWordWrap(char *word, int *x, int *y, int startX)
{
	int i, w, c;
	
	w = 0;
	
	for (i = 0 ; i < strlen(word) ; i++)
	{
		c = word[i];
		
		w += activeFont->glyphs[c].rect.w * scale;
		
		if (w + *x - startX > app.textWidth)
		{
			*x = startX;
			
			*y += activeFont->glyphs[c].rect.h * scale;
			
			return;
		}
	}
}

void useFont(char *name)
{
	Font *f;
	
	for (f = fontHead.next ; f != NULL ; f = f->next)
	{
		if (strcmp(f->name, name) == 0)
		{
			activeFont = f;
			return;
		}
	}
}

void calcTextDimensions(char *text, int size, int *w, int *h)
{
	int i, c;
	float scale;
	
	scale = size / (FONT_SIZE * 1.0f);
	
	*w = 0;
	*h = 0;
	
	for (i = 0 ; i < strlen(text) ; i++)
	{
		c = text[i];
		
		*w += activeFont->glyphs[c].rect.w * scale;
		*h = MAX(activeFont->glyphs[c].rect.h * scale, *h);
	}
}

int getWrappedTextHeight(char *text, int size)
{
	int x, y, w, h, i, c;
	float scale;
	
	scale = size / (FONT_SIZE * 1.0f);
	
	x = y = 0;
	
	for (i = 0 ; i < strlen(text) ; i++)
	{
		c = text[i];
		
		w = activeFont->glyphs[c].rect.w * scale;
		h = activeFont->glyphs[c].rect.h * scale;
		
		if (x + w > app.textWidth)
		{
			x = 0;
			y += h;
		}
		
		x += w;
	}
	
	return y;
}
