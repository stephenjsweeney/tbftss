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

#include "credits.h"

static void loadCredits(void);
static void logic(void);
static void draw(void);
static void handleKeyboard(void);

static SDL_Texture *background;
static SDL_Texture *earthTexture;
static Credit head;
static Credit *tail;
static float creditSpeed;
static int timeout;

void initCredits(void)
{
	startSectionTransition();
	
	stopMusic();
	
	memset(&head, 0, sizeof(Credit));
	tail = &head;
	
	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	background = getTexture("gfx/backgrounds/background02.jpg");
	
	earthTexture = getTexture("gfx/planets/earth.png");
	
	loadCredits();
	
	app.hideMouse = 1;
	
	endSectionTransition();
	
	playMusic("music/main/Her Violet Eyes.mp3", 0);
}

static void logic(void)
{
	Credit *c;
	
	handleKeyboard();
	
	for (c = head.next ; c != NULL ; c = c->next)
	{
		c->y -= creditSpeed;
		
		if (!c->next)
		{
			c->y = MAX(c->y, (SCREEN_HEIGHT - c->h) / 2);
		}
	}
	
	if (--timeout <= 0)
	{
		app.hideMouse = 0;
		
		initTitle();
	}
}

static void draw(void)
{
	Credit *c;
	
	drawBackground(background);
	
	blit(earthTexture, SCREEN_WIDTH - 200, (SCREEN_HEIGHT / 2) + 100, 1);
	
	limitTextWidth(CREDIT_LINE_LIMIT);
	
	for (c = head.next ; c != NULL ; c = c->next)
	{
		if (c->y > -c->h && c->y < SCREEN_HEIGHT)
		{
			drawText(SCREEN_WIDTH / 2, (int)c->y, c->size, TA_CENTER, colors.white, c->text);
		}
	}
	
	limitTextWidth(0);
}

static void loadCredits(void)
{
	cJSON *root, *node;
	int y, dist;
	char *text;
	Credit *c;
	
	y = SCREEN_HEIGHT + 100;

	text = readFile("data/credits/credits.json");
	root = cJSON_Parse(text);
	
	limitTextWidth(CREDIT_LINE_LIMIT);

	for (node = root->child ; node != NULL ; node = node->next)
	{
		c = malloc(sizeof(Credit));
		memset(c, 0, sizeof(Credit));
		tail->next = c;
		tail = c;
		
		c->y = y;
		
		c->text = malloc(sizeof(char) * strlen(node->valuestring));
		memset(c->text, '\0', sizeof(char) * strlen(node->valuestring));
		
		sscanf(node->valuestring, "%d %d %1023[^\n]", &dist, &c->size, c->text);
		
		c->y += dist;
		c->h = getWrappedTextHeight(c->text, c->size);
		
		y += c->h + dist;
	}
	
	limitTextWidth(0);
	
	/* the music that plays over the credits is 2m 44s, so scroll credits roughly inline with that (plus 2 seconds) */
	timeout = ((2 * 60) + 46) * FPS;
	
	creditSpeed = y;
	creditSpeed /= timeout;
	
	cJSON_Delete(root);
	free(text);
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		timeout = 0;
	}
}

void destroyCredits(void)
{
	Credit *c;
	
	while (head.next)
	{
		c = head.next;
		head.next = c->next;
		if (c->text)
		{
			free(c->text);
		}
		free(c);
	}
}
