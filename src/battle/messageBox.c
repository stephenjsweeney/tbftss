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

#include "messageBox.h"

static void calculateMessageBoxHeight(MessageBox *msg);
static void nextMessage(void);

static MessageBox head;
static MessageBox *tail;

void initMessageBox(void)
{
	memset(&head, 0, sizeof(MessageBox));
	tail = &head;
}

void addMessageBox(char *title, char *body, int important)
{
	MessageBox *msg;
	int isFirst;
	float time;
	
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
	msg->important = important;
	
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
	limitTextWidth(575);
	
	msg->height = getWrappedTextHeight(msg->body, 18);
	
	limitTextWidth(0);
}

int showingMessageBoxes(void)
{
	return head.next != NULL;
}

void drawMessageBox(void)
{
	MessageBox *msg = head.next;
	SDL_Rect r;
	
	if (msg && msg->time > 0)
	{
		if (!msg->height)
		{
			calculateMessageBoxHeight(msg);
		}
		
		r.y = 50;
		r.w = 650;
		r.h = msg->height + 40;
		r.x = (SCREEN_WIDTH - r.w) / 2;
		
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		
		if (!msg->important)
		{
			SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		}
		else
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 64);
		}
		SDL_RenderFillRect(app.renderer, &r);
		
		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 128);
		SDL_RenderDrawRect(app.renderer, &r);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		
		drawText(r.x + 10, r.y + 5, 18, TA_LEFT, colors.cyan, msg->title);
		
		limitTextWidth(575);
		
		drawText(r.x + 10, r.y + 30, 18, TA_LEFT, (!msg->important) ? colors.white : colors.red, msg->body);
		
		limitTextWidth(0);
	}
}

static void nextMessage(void)
{
	Entity *e;
	
	playSound(SND_RADIO);
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (strcmp(e->name, head.next->title) == 0)
		{
			battle.messageSpeaker = e;
			return;
		}
	}
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
