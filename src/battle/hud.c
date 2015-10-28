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

#include "hud.h"

static void drawHealthShieldBar(int current, int max, int x, int y, int r, int g, int b);
static void drawPlayerTargeter(void);
static void drawNumFighters(void);
static void drawHealthBars(void);
static void drawWeaponInfo(void);
static void drawObjectives(void);
static void drawTargetDistance(void);
static void drawMissionTargetDistance(void);
static void drawHudMessages(void);

static HudMessage hudMessageHead;
static HudMessage *hudMessageTail;
static SDL_Texture *targetPointer;
static SDL_Texture *targetCircle;
static SDL_Texture *smallFighter;
static int numMessages;
static int healthWarning;
static char *gunName[] = {"", "Particle Cannon", "Plasma Cannon", "Laser Cannon", "Mag Cannon"};

void initHud(void)
{
	healthWarning = 0;
	
	memset(&hudMessageHead, 0, sizeof(HudMessage));
	hudMessageTail = &hudMessageHead;
	
	targetPointer = getTexture("gfx/hud/targetPointer.png");
	targetCircle = getTexture("gfx/hud/targetCircle.png");
	smallFighter = getTexture("gfx/hud/smallFighter.png");
}

void doHud(void)
{
	HudMessage *hudMessage, *prev;
	
	healthWarning++;
	healthWarning %= FPS;
	
	numMessages = 0;
	
	prev = &hudMessageHead;
	
	for (hudMessage = hudMessageHead.next ; hudMessage != NULL ; hudMessage = hudMessage->next)
	{
		hudMessage->life--;
		
		numMessages++;
		
		if (hudMessage->life <= 0)
		{
			if (hudMessage == hudMessageTail)
			{
				hudMessageTail = prev;
			}
			
			prev->next = hudMessage->next;
			free(hudMessage);
			hudMessage = prev;
			
			numMessages--;
		}
		
		prev = hudMessage;
	}
}

void addHudMessage(SDL_Color c, char *format, ...)
{
	va_list args;
	
	HudMessage *hudMessage = malloc(sizeof(HudMessage));
	memset(hudMessage, 0, sizeof(HudMessage));
	hudMessageTail->next = hudMessage;
	hudMessageTail = hudMessage;

	va_start(args, format);
	vsprintf(hudMessageTail->message, format, args);
	va_end(args);
	
	hudMessage->color = c;
	hudMessage->life = FPS * 5;
	
	numMessages++;
	
	while (numMessages > MAX_HUD_MESSAGES)
	{
		hudMessage = hudMessageHead.next;
		hudMessageHead.next = hudMessage->next;
		free(hudMessage);
		
		numMessages--;
	}
}

void drawHud(void)
{
	if (player != NULL)
	{
		drawHealthBars();
		
		drawWeaponInfo();
		
		drawNumFighters();
		
		drawObjectives();
		
		drawRadar();
	}
	
	drawHudMessages();
}

static void drawHealthBars(void)
{
	float p;
	int r, g, b;
	
	drawPlayerTargeter();
	
	r = g = b = 0;
	p = player->health;
	p /= player->maxHealth;
	
	if (p <= 0.25)
	{
		r = 255;
	}
	else if (p <= 0.5)
	{
		r = 255;
		g = 255;
	}
	else
	{
		g = 200;
	}
	
	drawHealthShieldBar(player->health, player->maxHealth, 10, 10, r, g, b);
	drawHealthShieldBar(player->shield, player->maxShield, 10, 30, 0, 200, 255);
	
	if (player->target)
	{
		drawHealthShieldBar(player->target->health, player->target->maxHealth, SCREEN_WIDTH - 260, 10, 0, 200, 0);
		drawHealthShieldBar(player->target->shield, player->target->maxShield, SCREEN_WIDTH - 260, 30, 0, 200, 255);
		drawTargetDistance();
	}
	
	if (battle.missionTarget)
	{
		drawMissionTargetDistance();
	}
}

static void drawHealthShieldBar(int current, int max, int x, int y, int r, int g, int b)
{
	SDL_Rect rect;
	float percent = 0;
	
	if (max > 0)
	{
		percent = current;
		percent /= max;
	}
		
	rect.x = x;
	rect.y = y;
	rect.w = 250;
	rect.h = 12;
	
	SDL_SetRenderDrawColor(app.renderer, r / 2, g / 2, b / 2, 255);
	SDL_RenderFillRect(app.renderer, &rect);
	
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(app.renderer, &rect);
	
	if (current > 0)
	{
		rect.x += 2;
		rect.y += 2;
		rect.w -= 4;
		rect.h -= 4;
		
		rect.w *= percent;
		
		SDL_SetRenderDrawColor(app.renderer, r, g, b, 255);
		SDL_RenderFillRect(app.renderer, &rect);
	}
}

static void drawWeaponInfo(void)
{
	drawText(10, 50, 14, TA_LEFT, colors.white, gunName[player->selectedGunType]);
	drawText(260, 50, 14, TA_RIGHT, colors.white, "Missiles (%d)", player->missiles.ammo);
}

static void drawPlayerTargeter(void)
{
	float angle;
	int x, y;
	
	if (player->target || battle.missionTarget)
	{
		if (player->target)
		{
			SDL_SetTextureColorMod(targetCircle, 255, 0, 0);
		}
		else
		{
			SDL_SetTextureColorMod(targetCircle, 0, 255, 0);
		}
		
		blit(targetCircle, player->x, player->y, 1);
	}
	
	if (player->target)
	{
		angle = getAngle(player->x, player->y, player->target->x, player->target->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		SDL_SetTextureColorMod(targetPointer, 255, 0, 0);
		
		blitRotated(targetPointer, x, y, angle);
	}
	
	if (battle.missionTarget)
	{
		angle = getAngle(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		SDL_SetTextureColorMod(targetPointer, 0, 255, 0);
		
		blitRotated(targetPointer, x, y, angle);
	}
}

static void drawNumFighters(void)
{
	/* Allies */
	SDL_SetTextureColorMod(smallFighter, 150, 200, 255);
	blit(smallFighter, 400, 15, 0);
	drawText(425, 11, 14, TA_LEFT, colors.white, "(%d)", battle.numAllies);
	
	/* Enemies */
	SDL_SetTextureColorMod(smallFighter, 255, 100, 100);
	blit(smallFighter, SCREEN_WIDTH - 410, 15, 0);
	drawText(SCREEN_WIDTH - 420, 11, 14, TA_RIGHT, colors.white, "(%d)", battle.numEnemies);
}

static void drawObjectives(void)
{
	drawText(SCREEN_WIDTH / 2, 10, 16, TA_CENTER, colors.white, "%d / %d", battle.numObjectivesComplete, battle.numObjectivesTotal);
}

static void drawTargetDistance(void)
{
	float distance;
	
	if (player->target != NULL)
	{
		distance = getDistance(player->x, player->y, player->target->x, player->target->y);
		distance /= 50;
		
		distance = (int)distance;
		distance /= 10;
		
		drawText(SCREEN_WIDTH - 15, 50, 14, TA_RIGHT, colors.red, "Target: %.2fkm", distance);
	}
}

static void drawMissionTargetDistance(void)
{
	float distance;
	
	if (battle.missionTarget != NULL)
	{
		distance = getDistance(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		distance /= 50;
		
		distance = (int)distance;
		distance /= 10;
		
		drawText(SCREEN_WIDTH - 15, 75, 14, TA_RIGHT, colors.green, "Objective: %.2fkm", distance);
	}
}

static void drawHudMessages(void)
{
	HudMessage *hudMessage;
	int y = SCREEN_HEIGHT - 25;
	
	for (hudMessage = hudMessageHead.next ; hudMessage != NULL ; hudMessage = hudMessage->next)
	{
		drawText(10, y, 14, TA_LEFT, hudMessage->color, hudMessage->message);
		
		y -= 25;
	}
}

void resetHud(void)
{
	HudMessage *hudMessage;
	
	while (hudMessageHead.next)
	{
		hudMessage = hudMessageHead.next;
		hudMessageHead.next = hudMessage->next;
		free(hudMessage);
	}
}
