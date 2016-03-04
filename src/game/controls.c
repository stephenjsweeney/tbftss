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

#include "controls.h"

int isKeyControl(int type)
{
	return app.keyboard[app.keyControls[type]];
}

void clearControl(int type)
{
	app.keyboard[app.keyControls[type]] = 0;
}

void initControlsDisplay(void)
{
	getWidget("keyMissile", "controls")->value = app.keyControls[CONTROL_MISSILE];
	getWidget("keyBoost", "controls")->value = app.keyControls[CONTROL_BOOST];
	getWidget("keyECM", "controls")->value = app.keyControls[CONTROL_ECM];
	getWidget("keyBrake", "controls")->value = app.keyControls[CONTROL_BRAKE];
	getWidget("keyRadar", "controls")->value = app.keyControls[CONTROL_RADAR];
	getWidget("keyTarget", "controls")->value = app.keyControls[CONTROL_TARGET];
	getWidget("keyGuns", "controls")->value = app.keyControls[CONTROL_GUNS];
	getWidget("keyPrevFighter", "controls")->value = app.keyControls[CONTROL_PREV_FIGHTER];
	getWidget("keyNextFighter", "controls")->value = app.keyControls[CONTROL_NEXT_FIGHTER];
}

void drawControls(void)
{
	SDL_Rect r;

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

	r.w = 800;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);

	drawText(SCREEN_WIDTH / 2, 50, 28, TA_CENTER, colors.white, _("Controls"));

	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, r.y + 65, r.x + r.w, r.y + 65);
	
	drawWidgets("controls");
}
