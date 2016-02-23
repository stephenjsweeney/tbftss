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

#include "modalDialog.h"

static Widget *ok;
static Widget *okCancelOK;
static Widget *okCancelCancel;

void initModalDialog(void)
{
	ok = getWidget("ok", "ok");
	okCancelOK = getWidget("ok", "okCancel");
	okCancelCancel = getWidget("cancel", "okCancel");
}

void doModalDialog(void)
{
	doWidgets();
	
	clearInput();
}

void drawModalDialog(void)
{
	SDL_Rect r;
	
	limitTextWidth(700);
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 96);
	SDL_RenderFillRect(app.renderer, NULL);
	
	r.w = 800;
	r.h = getWrappedTextHeight(app.modalDialog.message, 24) + 100;
	r.x = (SCREEN_WIDTH / 2) - (r.w / 2);
	r.y = (SCREEN_HEIGHT / 2) - (r.h / 2);
	
	ok->rect.y = okCancelOK->rect.y = okCancelCancel->rect.y = r.y + r.h - 50;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 225);
	SDL_RenderFillRect(app.renderer, &r);
	
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 200);
	SDL_RenderDrawRect(app.renderer, &r);
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	drawText(SCREEN_WIDTH / 2, r.y + 10, 24, TA_CENTER, colors.white, app.modalDialog.message);
	
	switch (app.modalDialog.type)
	{
		case MD_OK:
			drawWidgets("ok");
			break;
			
		case MD_OK_CANCEL:
			drawWidgets("okCancel");
			break;
	}
	
	limitTextWidth(0);
}
