/*
Copyright (C) 2018-2019 Parallel Realities

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
static void drawTextLines(int x, int y, int size, int align);
static void drawTextLine(int x, int y, int size, int align, const char *line);
void calcTextDimensions(const char *text, int size, int *w, int *h);
void useFont(char *name);
static void initChars(Font *f);
static char *nextCharacter(const char *str, int *i);
static Glyph *findGlyph(char *c);

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
	Glyph *g;
	int i;
	SDL_Color white = {255, 255, 255, 255};

	f = malloc(sizeof(Font));
	memset(f, 0, sizeof(Font));

	font = TTF_OpenFont(filename, FONT_SIZE);

	initChars(f);

	surface = SDL_CreateRGBSurface(0, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 32, 0, 0, 0, 0xff);

	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

	dest.x = dest.y = 0;

	for (i = 0 ; i < NUM_GLYPH_BUCKETS ; i++)
	{
		for (g = f->glyphHead[i].next ; g != NULL ; g = g->next)
		{
			text = TTF_RenderUTF8_Blended(font, g->character, white);

			TTF_SizeText(font, g->character, &dest.w, &dest.h);

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

			g->rect = dest;

			SDL_FreeSurface(text);

			dest.x += dest.w;
		}
	}

	TTF_CloseFont(font);

	texture = toTexture(surface, 1);

	f->texture = texture;

	strcpy(f->name, name);

	fontTail->next = f;
	fontTail = f;
}

static void initChars(Font *f)
{
	char *characters, *character;
	Glyph *g, *glyphTail;
	int i, bucket;

	characters = readFile("data/locale/characters.dat");

	i = 0;

	character = nextCharacter(characters, &i);

	while (character)
	{
		bucket = hashcode(character) % NUM_GLYPH_BUCKETS;

		glyphTail = &f->glyphHead[bucket];

		/* horrible bit to look for the tail */
		while (glyphTail->next)
		{
			glyphTail = glyphTail->next;
		}

		g = malloc(sizeof(Glyph));
		memset(g, 0, sizeof(Glyph));
		glyphTail->next = g;
		glyphTail = g;

		STRNCPY(g->character, character, MAX_NAME_LENGTH);

		character = nextCharacter(characters, &i);
	}

	free(characters);
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
	int i;
	char *character;
	SDL_Rect dest;
	Glyph *g;

	i = 0;

	character = nextCharacter(word, &i);

	while (character)
	{
		g = findGlyph(character);

		dest.x = *x;
		dest.y = *y;
		dest.w = g->rect.w * scale;
		dest.h = g->rect.h * scale;

		SDL_RenderCopy(app.renderer, activeFont->texture, &g->rect, &dest);

		*x += g->rect.w * scale;

		character = nextCharacter(word, &i);
	}
}

static Glyph *findGlyph(char *c)
{
	Glyph *g;
	int bucket;

	bucket = hashcode(c) % NUM_GLYPH_BUCKETS;

	for (g = activeFont->glyphHead[bucket].next ; g != NULL ; g = g->next)
	{
		if (strcmp(g->character, c) == 0)
		{
			return g;
		}
	}

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, "Couldn't find glyph for '%s'", c);
	exit(1);

	return NULL;
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
	int i;
	char *character;
	Glyph *g;

	scale = size / (FONT_SIZE * 1.0f);

	*w = 0;
	*h = 0;

	i = 0;

	character = nextCharacter(text, &i);

	while (character)
	{
		g = findGlyph(character);

		*w += g->rect.w * scale;
		*h = MAX(g->rect.h * scale, *h);

		character = nextCharacter(text, &i);
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

static char *nextCharacter(const char *str, int *i)
{
	static char character[MAX_NAME_LENGTH];

	unsigned char bit;
	int n, numBits;

	memset(character, '\0', MAX_NAME_LENGTH);

	bit = (unsigned char)str[*i];

	numBits = 0;

	if (bit == '\0')
	{
		return NULL;
	}
	else if (bit <= 0x0000007F)
	{
		numBits = 1;
	}
	else if (bit <= 0x000007FF)
	{
		numBits = 2;
	}
	else if (bit <= 0x0000FFFF)
	{
		numBits = 3;
	}
	else if (bit <= 0x001FFFFF)
	{
		numBits = 4;
	}
	else if (bit <= 0x03FFFFFF)
	{
		numBits = 5;
	}
	else if (bit <= 0x7FFFFFFF)
	{
		numBits = 6;
	}

	for (n = 0 ; n < numBits ; n++)
	{
		character[n] = str[(*i)++];
	}

	return character;
}
