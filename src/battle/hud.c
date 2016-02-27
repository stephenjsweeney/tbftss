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

#include "hud.h"

static void drawPlayerTargeter(void);
static void drawNumFighters(void);
static void drawHealthBars(void);
static void drawWeaponInfo(void);
static void drawObjectives(void);
static void drawDistancesInfo(void);
static void drawHudMessages(void);
static void drawPlayerSelect(void);
static void drawAbilityBars(void);
static void drawBoostECMBar(int current, int max, int x, int y, int r, int g, int b);
static void drawHealthShieldBar(int current, int max, int x, int y, int r, int g, int b, int flashLow);

static HudMessage hudMessageHead;
static HudMessage *hudMessageTail;
static SDL_Texture *targetPointer;
static SDL_Texture *targetCircle;
static SDL_Texture *smallFighter;
static SDL_Texture *arrowLeft;
static SDL_Texture *arrowRight;
static SDL_Texture *armour;
static SDL_Texture *shield;
static SDL_Texture *ecm;
static SDL_Texture *boost;
static SDL_Texture *nextGun;
static int numMessages;
static char *gunName[] = {"", "Particle Cannon", "Plasma Cannon", "Laser Cannon", "Mag Cannon", "Rockets", "Missiles"};

void initHud(void)
{
	memset(&hudMessageHead, 0, sizeof(HudMessage));
	hudMessageTail = &hudMessageHead;
	
	targetPointer = getTexture("gfx/hud/targetPointer.png");
	targetCircle = getTexture("gfx/hud/targetCircle.png");
	smallFighter = getTexture("gfx/hud/smallFighter.png");
	arrowLeft = getTexture("gfx/widgets/optionsLeft.png");
	arrowRight = getTexture("gfx/widgets/optionsRight.png");
	armour = getTexture("gfx/hud/armour.png");
	shield = getTexture("gfx/hud/shield.png");
	ecm = getTexture("gfx/hud/ecm.png");
	boost = getTexture("gfx/hud/boost.png");
	nextGun = getTexture("gfx/hud/nextGun.png");
}

void doHud(void)
{
	HudMessage *hudMessage, *prev;
	
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
		
		drawAbilityBars();
		
		drawPlayerTargeter();
		
		drawWeaponInfo();
		
		drawNumFighters();
		
		drawObjectives();
		
		drawDistancesInfo();
		
		drawRadar();
		
		drawRadarRangeWarning();
	}
	
	drawHudMessages();
	
	if (battle.playerSelect)
	{
		drawPlayerSelect();
	}
}

static void drawHealthBars(void)
{
	float p;
	int r, g, b;
	
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
	
	blit(armour, 6, 9, 0);
	drawHealthShieldBar(player->health, player->maxHealth, 30, 10, r, g, b, 1);
	
	blit(shield, 6, 29, 0);
	drawHealthShieldBar(player->shield, player->maxShield, 30, 30, 0, 200, 255, 0);
}

static void drawHealthShieldBar(int current, int max, int x, int y, int r, int g, int b, int flashLow)
{
	SDL_Rect rect;
	float percent = 0;
	
	if (max > 0)
	{
		percent = current;
		percent /= max;
		
		if (flashLow && percent <= 0.25 && battle.stats[STAT_TIME] % FPS < 30)
		{
			percent = 0;
		}
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

static void drawAbilityBars(void)
{
	blit(boost, 6, 49, 0);
	drawBoostECMBar(battle.boostTimer, BOOST_RECHARGE_TIME, 30, 50, 128, 128, 255);
	
	blit(ecm, 155, 49, 0);
	drawBoostECMBar(battle.ecmTimer, ECM_RECHARGE_TIME, 175, 50, 255, 128, 0);
}

static void drawBoostECMBar(int current, int max, int x, int y, int r, int g, int b)
{
	SDL_Rect rect;
	
	float percent = current;
	percent /= max;
	
	rect.x = x;
	rect.y = y;
	rect.w = 105;
	rect.h = 12;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(app.renderer, &rect);
	
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(app.renderer, &rect);
	
	rect.x += 2;
	rect.y += 2;
	rect.w -= 4;
	rect.h -= 4;
	
	rect.w *= percent;
	
	if (current < max)
	{
		r /= 2;
		g /= 2;
		b /= 2;
	}
	
	SDL_SetRenderDrawColor(app.renderer, r, g, b, 255);
	SDL_RenderFillRect(app.renderer, &rect);
}

static void drawWeaponInfo(void)
{
	int i, y;
	
	if (!player->combinedGuns)
	{
		if (battle.numPlayerGuns)
		{
			y = 70;
			
			for (i = 0 ; i < BT_MAX ; i++)
			{
				if (playerHasGun(i))
				{
					if (player->selectedGunType == i)
					{
						drawText(30, y, 14, TA_LEFT, colors.green, "%s", gunName[i]);
						
						blit(nextGun, 8, y + 5, 0);
					}
					else
					{
						drawText(30, y, 14, TA_LEFT, colors.darkGrey, "%s", gunName[i]);
					}
					
					y += 20;
				}
			}
		}
		else
		{
			drawText(30, 70, 14, TA_LEFT, colors.white, "(None)");
		}
	}
	else
	{
		drawText(30, 70, 14, TA_LEFT, colors.white, "(Combined Guns)");
	}
	
	drawText(280, 70, 14, TA_RIGHT, colors.white, "Missiles (%d)", player->missiles);
}

static void drawPlayerTargeter(void)
{
	float angle;
	int x, y;
	
	if (player->target || battle.missionTarget || battle.extractionPoint)
	{
		if (player->target)
		{
			SDL_SetTextureColorMod(targetCircle, 255, 0, 0);
		}
		else if (battle.missionTarget)
		{
			SDL_SetTextureColorMod(targetCircle, 0, 255, 0);
		}
		else
		{
			SDL_SetTextureColorMod(targetCircle, 255, 255, 0);
		}
		
		blit(targetCircle, player->x - battle.camera.x, player->y - battle.camera.y, 1);
	}
	
	if (player->target)
	{
		angle = getAngle(player->x, player->y, player->target->x, player->target->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		SDL_SetTextureColorMod(targetPointer, 255, 0, 0);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
	
	if (battle.missionTarget)
	{
		angle = getAngle(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		SDL_SetTextureColorMod(targetPointer, 0, 255, 0);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
	
	if (battle.extractionPoint)
	{
		angle = getAngle(player->x, player->y, battle.extractionPoint->x, battle.extractionPoint->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		SDL_SetTextureColorMod(targetPointer, 255, 255, 0);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
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
	if (!battle.isChallenge)
	{
		drawText(SCREEN_WIDTH / 2, 10, 16, TA_CENTER, colors.white, "%d / %d", battle.numObjectivesComplete, battle.numObjectivesTotal);
	}
	else
	{
		drawText(SCREEN_WIDTH / 2, 10, 16, TA_CENTER, colors.white, "%d", battle.stats[STAT_TIME] / FPS);
	}
}

static float distanceToKM(int x1, int y1, int x2, int y2)
{
	float distance;
	
	distance = getDistance(x1, y1, x2, y2);
	
	/* 2px = 1m approx */
	distance /= 2;
	distance = (int)distance;
	distance /= 1000;
	
	return distance;
}

static void drawDistancesInfo(void)
{
	int y;
	float distance;
	
	y = 11;
	
	if (player->target != NULL)
	{
		drawText(SCREEN_WIDTH - 15, y, 18, TA_RIGHT, colors.red, player->target->name);
		
		y += 30;
		
		distance = distanceToKM(player->x, player->y, player->target->x, player->target->y);
		
		drawText(SCREEN_WIDTH - 15, y, 14, TA_RIGHT, colors.red, "Target: %.2fkm", distance);
		
		y += 25;
	}
	
	if (battle.missionTarget != NULL)
	{
		distance = distanceToKM(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		
		drawText(SCREEN_WIDTH - 15, y, 14, TA_RIGHT, colors.green, "Objective: %.2fkm", distance);
		
		y += 25;
	}
	
	if (battle.extractionPoint != NULL)
	{
		distance = distanceToKM(player->x, player->y, battle.extractionPoint->x, battle.extractionPoint->y);
		
		drawText(SCREEN_WIDTH - 15, y, 14, TA_RIGHT, colors.yellow, "Extraction Point: %.2fkm", distance);
		
		y += 25;
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

static void drawPlayerSelect(void)
{
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	SDL_SetTextureColorMod(targetCircle, 0, 200, 255);
	
	blit(targetCircle, player->x - battle.camera.x, player->y - battle.camera.y, 1);
	
	drawText(SCREEN_WIDTH / 2, 500, 28, TA_CENTER, colors.white, "SELECT NEW FIGHTER");
	
	if (player->health > 0)
	{
		drawText(SCREEN_WIDTH / 2, 540, 20, TA_CENTER, colors.white, "%s (%d%% / %d%%)", player->defName, getPercent(player->health, player->maxHealth), getPercent(player->shield, player->maxShield));
	}
	
	blit(arrowLeft, (SCREEN_WIDTH / 2) - 200, 520, 1);
	blit(arrowRight, (SCREEN_WIDTH / 2) + 200, 520, 1);
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
