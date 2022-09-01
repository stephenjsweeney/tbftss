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

#include "../system/sound.h"
#include "../system/text.h"
#include "messageBox.h"

#define MSG_BOX_TEXT_WIDTH 600

extern App     app;
extern Battle  battle;
extern Colors  colors;
extern Entity *player;

static void calculateMessageBoxHeight(MessageBox *msg);
static void nextMessage(void);

static MessageBox  head;
static MessageBox *tail;
static Entity     *lastWingmate;

void initMessageBox(void)
{
	memset(&head, 0, sizeof(MessageBox));
	tail = &head;

	lastWingmate = NULL;
}

void addMessageBox(char *title, char *body, int type)
{
	MessageBox *msg;
	int         isFirst;
	float       time;

	isFirst = (tail == &head);

	msg = malloc(sizeof(MessageBox));
	memset(msg, 0, sizeof(MessageBox));
	tail->next = msg;
	tail = msg;

	time = 0.075 * strlen(body);
	time = MIN(MAX(time, 3), 7);

	STRNCPY(msg->title, title, MAX_NAME_LENGTH);
	STRNCPY(msg->body, body, MAX_DESCRIPTION_LENGTH);
	msg->time = time * FPS;
	msg->type = type;

	if (isFirst)
	{
		nextMessage();
	}
}

void doMessageBox(void)
{
	MessageBox *msg;

	msg = head.next;

	if (msg)
	{
		if (--msg->time <= -(FPS / 4))
		{
			if (msg == tail)
			{
				tail = &head;
			}

			head.next = msg->next;
			free(msg);
			msg = &head;

			battle.messageSpeaker = NULL;

			if (head.next)
			{
				nextMessage();
			}
		}
	}
}

static void calculateMessageBoxHeight(MessageBox *msg)
{
	app.textWidth = MSG_BOX_TEXT_WIDTH;

	if (msg->type == MB_PANDORAN)
	{
		useFont("khosrau");
	}
	else
	{
		useFont("roboto");
	}

	msg->height = getWrappedTextHeight(msg->body, 18);

	app.textWidth = 0;

	useFont("roboto");
}

int showingMessageBoxes(void)
{
	return head.next != NULL;
}

void drawMessageBox(void)
{
	MessageBox *msg = head.next;
	SDL_Rect    r;

	if (msg && msg->time > 0)
	{
		if (!msg->height)
		{
			calculateMessageBoxHeight(msg);
		}

		r.y = 50;
		r.w = 650;
		r.h = msg->height + 40;
		r.x = (app.winWidth - r.w) / 2;

		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);

		if (msg->type == MB_IMPORTANT)
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 64);
		}
		else
		{
			SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		}
		SDL_RenderFillRect(app.renderer, &r);

		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 128);
		SDL_RenderDrawRect(app.renderer, &r);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

		drawText(r.x + 10, r.y + 5, 18, TA_LEFT, colors.cyan, msg->title);

		app.textWidth = MSG_BOX_TEXT_WIDTH;

		if (msg->type == MB_PANDORAN)
		{
			useFont("khosrau");
		}
		else
		{
			useFont("roboto");
		}

		drawText(r.x + 10, r.y + 30, 18, TA_LEFT, (msg->type != MB_IMPORTANT) ? colors.white : colors.red, msg->body);

		app.textWidth = 0;
	}

	useFont("roboto");
}

static void nextMessage(void)
{
	Entity *e, *wingmate;
	int     isWingmate;

	wingmate = NULL;

	isWingmate = strcmp(head.next->title, "Wingmate") == 0;

	playSound(SND_RADIO);

	for (e = battle.entityHead.next; e != NULL; e = e->next)
	{
		if (e->active && e != player)
		{
			if (strcmp(e->name, head.next->title) == 0)
			{
				battle.messageSpeaker = lastWingmate = e;
				return;
			}

			if (isWingmate && e->side == player->side && e->type == ET_FIGHTER && e->speed > 0)
			{
				wingmate = e;

				if (rand() % 2 && e != lastWingmate)
				{
					battle.messageSpeaker = lastWingmate = e;
					return;
				}
			}
		}
	}

	battle.messageSpeaker = wingmate;
}

void resetMessageBox(void)
{
	MessageBox *messageBox;

	while (head.next)
	{
		messageBox = head.next;
		head.next = messageBox->next;
		free(messageBox);
	}

	tail = &head;
}
