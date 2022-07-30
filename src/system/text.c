/*
Copyright (C) 2018-2019,2022 Parallel Realities

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
#include "text.h"
#include <SDL2/SDL_ttf.h>
#include "../system/textures.h"
#include "../system/io.h"

#define FONT_SIZE            32
#define FONT_TEXTURE_SIZE    512
#define MAX_WORD_LENGTH      128
#define MAX_GLYPH_SIZE       8

extern App app;

static void initFont(char *name, char *filename, char *characters);
static void drawWord(char *word, int *x, int *y, int startX);
static void drawTextLines(int x, int y, int size, int align);
static void drawTextLine(int x, int y, int size, int align, const char *line);
static int nextGlyph(const char *str, int *i, char *glyphBuffer);

static char drawTextBuffer[1024];
static Font fontHead;
static Font *fontTail;
static Font *activeFont = NULL;
static float scale;

void initFonts(void)
{
	char *characters;

	memset(&fontHead, 0, sizeof(Font));
	fontTail = &fontHead;

	characters = readFile("data/locale/characters.dat");

	initFont("roboto", getFileLocation("data/fonts/Roboto-Medium.ttf"), characters);

	initFont("khosrau", getFileLocation("data/fonts/Khosrau.ttf"), characters);

	useFont("roboto");

	free(characters);
}

static void initFont(char *name, char *filename, char *characters)
{
	SDL_Texture *texture;
	TTF_Font *font;
	Font *f;
	SDL_Surface *surface, *text;
	SDL_Rect dest;
	int i, n, largest;
	char glyphBuffer[MAX_GLYPH_SIZE];
	SDL_Color white = {255, 255, 255, 255};

	f = malloc(sizeof(Font));
	memset(f, 0, sizeof(Font));

	font = TTF_OpenFont(filename, FONT_SIZE);

	surface = SDL_CreateRGBSurface(0, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 32, 0, 0, 0, 0xff);

	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

	dest.x = dest.y = 0;

	largest = 0;

	i = 0;

	while ((n = nextGlyph(characters, &i, glyphBuffer)) != 0)
	{
		largest = MAX(largest, n);

		text = TTF_RenderUTF8_Blended(font, glyphBuffer, white);

		TTF_SizeText(font, glyphBuffer, &dest.w, &dest.h);

		if (dest.x + dest.w >= FONT_TEXTURE_SIZE)
		{
			dest.x = 0;

			dest.y += dest.h + 1;

			if (dest.y + dest.h >= FONT_TEXTURE_SIZE)
			{
				SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, "Out of glyph space in %dx%d font atlas texture map.", FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE);
				exit(1);
			}
		}

		SDL_BlitSurface(text, NULL, surface, &dest);

		f->glyphs[n] = dest;

		SDL_FreeSurface(text);

		dest.x += dest.w;
	}

	TTF_CloseFont(font);

	texture = toTexture(surface, 1);

	f->texture = texture;

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
			drawTextLine(x, y, size, align, drawTextBuffer);
		}
		else
		{
			drawTextLines(x, y, size, align);
		}
	}
}

static void drawTextLines(int x, int y, int size, int align)
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
				drawTextLine(x, y, size, align, line);

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

	drawTextLine(x, y, size, align, line);
}

static void drawTextLine(int x, int y, int size, int align, const char *line)
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
	int i, n;
	SDL_Rect dest;

	i = 0;

	while ((n = nextGlyph(word, &i, NULL)) != 0)
	{
		dest.x = *x;
		dest.y = *y;
		dest.w = activeFont->glyphs[n].w * scale;
		dest.h = activeFont->glyphs[n].h * scale;

		SDL_RenderCopy(app.renderer, activeFont->texture, &activeFont->glyphs[n], &dest);

		*x += activeFont->glyphs[n].w * scale;
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
	float scale;
	int i, n;

	scale = size / (FONT_SIZE * 1.0f);

	*w = 0;
	*h = 0;

	i = 0;

	while ((n = nextGlyph(text, &i, NULL)) != 0)
	{
		*w += activeFont->glyphs[n].w * scale;
		*h = MAX(activeFont->glyphs[n].h * scale, *h);
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

static int nextGlyph(const char *str, int *i, char *glyphBuffer)
{
	int len;
	unsigned bit;
	const char *p;

	bit = (unsigned char) str[*i];

	if (bit < ' ')
	{
		return 0;
	}

	len = 1;

	if (bit >= 0xF0)
	{
		bit  = (int)(str[*i]     & 0x07) << 18;
		bit |= (int)(str[*i + 1] & 0x3F) << 12;
		bit |= (int)(str[*i + 2] & 0x3F) << 6;
		bit |= (int)(str[*i + 3] & 0x3F);

		len = 4;
	}
	else if (bit >= 0xE0)
	{
		bit  = (int)(str[*i]     & 0x0F) << 12;
		bit |= (int)(str[*i + 1] & 0x3F) << 6;
		bit |= (int)(str[*i + 2] & 0x3F);

		len = 3;
	}
	else if (bit >= 0xC0)
	{
		bit  = (int)(str[*i]     & 0x1F) << 6;
		bit |= (int)(str[*i + 1] & 0x3F);

		len = 2;
	}

	/* only fill the buffer if it's been supplied */
	if (glyphBuffer != NULL)
	{
		p = str + *i;

		memset(glyphBuffer, 0, MAX_GLYPH_SIZE);

		memcpy(glyphBuffer, p, len);

		if (bit >= MAX_GLYPHS)
		{
			printf("Glyph '%s' index exceeds array size (%d >= %d)\n", glyphBuffer, bit, MAX_GLYPHS);
			exit(1);
		}
	}

	*i = *i + len;

	return bit;
}
