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

#include "messageBox.h"

static MessageBox head;
static MessageBox *tail;

void initMessageBox(void)
{
	memset(&head, 0, sizeof(MessageBox));
	tail = &head;
}

void addMessageBox(int time, char *title, char *body)
{
	MessageBox *msg = malloc(sizeof(MessageBox));
	memset(msg, 0, sizeof(MessageBox));
	tail->next = msg;
	tail = msg;
	
	STRNCPY(msg->title, title, MAX_NAME_LENGTH);
	STRNCPY(msg->body, body, MAX_DESCRIPTION_LENGTH);
	msg->time = time * FPS;
}

void doMessageBox(void)
{
	MessageBox *msg, *prev;
	
	msg = head.next;
	prev = &head;
	
	if (msg)
	{
		if (--msg->time <= -FPS)
		{
			prev->next = msg->next;
			free(msg);
			msg = prev;
		}
		
		prev = msg;
	}
}

void drawMessageBox(void)
{
	MessageBox *msg = head.next;
	SDL_Rect r;
	
	if (msg && msg->time > 0)
	{
		r.y = 50;
		r.w = 650;
		r.h = 110;
		r.x = (SCREEN_WIDTH - r.w) / 2;
		
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		SDL_RenderFillRect(app.renderer, &r);
		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 128);
		SDL_RenderDrawRect(app.renderer, &r);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		
		drawText(r.x + 10, r.y + 5, 18, TA_LEFT, colors.cyan, msg->title);
		
		limitTextWidth(600);
		
		drawText(r.x + 10, r.y + 30, 18, TA_LEFT, colors.white, msg->body);
		
		limitTextWidth(0);
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
