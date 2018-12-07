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
static void drawTextLines(int x, int y, int size, int align, SDL_Color color);
static void drawTextLine(int x, int y, int size, int align, SDL_Color color, const char *line);
void calcTextDimensions(const char *text, int size, int *w, int *h);
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
	va_list args;
	
	if (activeFont)
	{
		SDL_SetTextureColorMod(activeFont->texture, color.r, color.g, color.b);
		SDL_SetTextureAlphaMod(activeFont->texture, color.a);
		
		memset(&drawTextBuffer, '\0', sizeof(drawTextBuffer));

		va_start(args, format);
		vsprintf(drawTextBuffer, format, args);
		va_end(args);
		
		if (app.textWidth == 0)
		{
			drawTextLine(x, y, size, align, color, drawTextBuffer);
		}
		else
		{
			drawTextLines(x, y, size, align, color);
		}
	}
}

static void drawTextLines(int x, int y, int size, int align, SDL_Color color)
{
	char line[MAX_LINE_LENGTH], token[MAX_WORD_LENGTH];
	int i, n, w, h, currentWidth, len;
	
	memset(&line, '\0', sizeof(line));
	memset(&token, '\0', sizeof(token));
	
	len = strlen(drawTextBuffer);
	
	n = currentWidth = 0;
	
	for (i = 0 ; i < len ; i++)
	{
		token[n++] = drawTextBuffer[i];
		
		if (drawTextBuffer[i] == ' ' || i == len - 1)
		{
			calcTextDimensions(token, size, &w, &h);
		
			if (currentWidth + w > app.textWidth)
			{
				drawTextLine(x, y, size, align, color, line);
				
				currentWidth = 0;
				
				y += h;
				
				memset(&line, '\0', sizeof(line));
			}
			
			strcat(line, token);
			
			n = 0;
			
			memset(&token, '\0', sizeof(token));
			
			currentWidth += w;
		}
	}
	
	drawTextLine(x, y, size, align, color, line);
}

static void drawTextLine(int x, int y, int size, int align, SDL_Color color, const char *line)
{
	int i, startX, n, w, h;
	char word[MAX_WORD_LENGTH];
	
	scale = size / (FONT_SIZE * 1.0f);
		
	startX = x;
	
	memset(word, 0, MAX_WORD_LENGTH);
	
	n = 0;
	
	calcTextDimensions(line, size, &w, &h);
	
	if (align == TA_RIGHT)
	{
		x -= w;
	}
	else if (align == TA_CENTER)
	{
		x -= (w / 2);
	}
	
	for (i = 0 ; i < strlen(line) ; i++)
	{
		word[n++] = line[i];
		
		if (line[i] == ' ')
		{
			drawWord(word, &x, &y, startX);
			
			memset(word, 0, MAX_WORD_LENGTH);
			
			n = 0;
		}
	}
	
	drawWord(word, &x, &y, startX);
}

static void drawWord(char *word, int *x, int *y, int startX)
{
	int i, c;
	SDL_Rect dest;
	
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

void calcTextDimensions(const char *text, int size, int *w, int *h)
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
	char word[MAX_WORD_LENGTH];
	int i, y, n, w, h, currentWidth, len;
	
	STRNCPY(drawTextBuffer, text, MAX_LINE_LENGTH);
	
	n = 0;
	y = 0;
	h = 0;
	currentWidth = 0;
	len = strlen(drawTextBuffer);
	memset(word, 0, MAX_WORD_LENGTH);
	
	for (i = 0 ; i < len ; i++)
	{
		word[n++] = drawTextBuffer[i];
		
		if (drawTextBuffer[i] == ' ' || i == len - 1)
		{
			calcTextDimensions(word, size, &w, &h);
		
			if (currentWidth + w > app.textWidth)
			{
				currentWidth = 0;
				y += h;
			}
			
			currentWidth += w;
			
			memset(word, 0, MAX_WORD_LENGTH);
			
			n = 0;
		}
	}
	
	return y + h;
}
