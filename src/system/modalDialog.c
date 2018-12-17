/*
Copyright (C) 2015-2018 Parallel Realities

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

#include "modalDialog.h"

static int enableTimer;
static Widget *ok;
static Widget *okCancelOK;
static Widget *okCancelCancel;
static char textBuffer[MAX_DESCRIPTION_LENGTH];

void initModalDialog(void)
{
	ok = getWidget("ok", "ok");
	ok->action = NULL;
	ok->isModal = 1;

	okCancelOK = getWidget("ok", "okCancel");
	okCancelOK->action = NULL;
	okCancelOK->isModal = 1;

	okCancelCancel = getWidget("cancel", "okCancel");
	okCancelCancel->action = NULL;
	okCancelCancel->isModal = 1;

	enableTimer = 0;
}

void showOKDialog(void (*callback)(void), const char *format, ...)
{
	va_list args;

	memset(&textBuffer, '\0', sizeof(textBuffer));

	va_start(args, format);
	vsprintf(textBuffer, format, args);
	va_end(args);

	STRNCPY(app.modalDialog.message, textBuffer, MAX_DESCRIPTION_LENGTH);

	app.modalDialog.type = MD_OK;

	ok->action = callback;

	enableTimer = FPS;
	ok->enabled = okCancelOK->enabled = okCancelCancel->enabled = 0;
}

void showOKCancelDialog(void (*okCallback)(void), void (*cancelCallback)(void), const char *format, ...)
{
	va_list args;

	memset(&textBuffer, '\0', sizeof(textBuffer));

	va_start(args, format);
	vsprintf(textBuffer, format, args);
	va_end(args);

	STRNCPY(app.modalDialog.message, textBuffer, MAX_DESCRIPTION_LENGTH);

	app.modalDialog.type = MD_OK_CANCEL;

	okCancelOK->action = okCallback;
	okCancelCancel->action = cancelCallback;

	enableTimer = FPS / 4;
	ok->enabled = okCancelOK->enabled = okCancelCancel->enabled = 0;
}

void doModalDialog(void)
{
	doWidgets();

	clearInput();

	enableTimer = MAX(enableTimer - 1, 0);
	if (!enableTimer)
	{
		ok->enabled = okCancelOK->enabled = okCancelCancel->enabled = 1;
	}
}

void drawModalDialog(void)
{
	SDL_Rect r;
	
	app.textWidth = 700;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 96);
	SDL_RenderFillRect(app.renderer, NULL);
	
	SDL_SetRenderTarget(app.renderer, app.uiBuffer);

	r.w = 800;
	r.h = getWrappedTextHeight(app.modalDialog.message, 24) + 100;
	r.x = (UI_WIDTH / 2) - (r.w / 2);
	r.y = (UI_HEIGHT / 2) - (r.h / 2);

	ok->rect.y = okCancelOK->rect.y = okCancelCancel->rect.y = r.y + r.h - 50;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 225);
	SDL_RenderFillRect(app.renderer, &r);

	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 200);
	SDL_RenderDrawRect(app.renderer, &r);

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

	drawText(UI_WIDTH / 2, r.y + 10, 24, TA_CENTER, colors.white, app.modalDialog.message);

	switch (app.modalDialog.type)
	{
		case MD_OK:
			drawWidgets("ok");
			break;

		case MD_OK_CANCEL:
			drawWidgets("okCancel");
			break;
	}

	app.textWidth = 0;
	
	SDL_SetRenderTarget(app.renderer, app.backBuffer);
}
