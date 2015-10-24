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

#include "stats.h"

void drawStats(void)
{
	int y, hours, minutes, seconds;
	SDL_Rect r;
	char timePlayed[MAX_NAME_LENGTH];
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = 500;
	r.h = 600;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 70, 28, TA_CENTER, colors.white, "Stats");
	
	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, 120, r.x + r.w, 120);
	
	y = 140;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Missions Started");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.missionsStarted);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Missions Completed");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.missionsCompleted);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Shots Fired");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.shotsFired);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Shots Hit");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.shotsHit);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Missiles Fired");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.missilesFired);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Missiles Hit");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.missilesHit);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Enemies Killed (Player)");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.playerKills);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Enemies Killed (All)");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.enemiesKilled);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Allies Lost");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.alliesKilled);
	y += 40;
	
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Times Killed");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, "%d", game.stats.playerKilled);
	y += 60;
	
	seconds = game.stats.time / FPS;
	minutes = (seconds / 60) % 60;
	hours = seconds / (60 * 60);
	
	seconds %= 60;
	
	sprintf(timePlayed, "%dh:%02dm:%02ds", hours, minutes, seconds);
	drawText(r.x + 20, y, 18, TA_LEFT, colors.white, "Time Played");
	drawText(r.x + r.w - 20, y, 18, TA_RIGHT, colors.white, timePlayed);
		
	drawWidgets("stats");	
}
