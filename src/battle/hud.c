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
static void drawSuspicionLevel(void);
static void drawMissileWarning(void);

static HudMessage hudMessageHead;
static HudMessage *hudMessageTail;
static AtlasImage *targetPointer;
static AtlasImage *targetCircle;
static AtlasImage *smallFighter;
static AtlasImage *arrowLeft;
static AtlasImage *arrowRight;
static AtlasImage *armour;
static AtlasImage *shield;
static AtlasImage *ecm;
static AtlasImage *boost;
static AtlasImage *nextGun;
static AtlasImage *clockIcon;
static AtlasImage *objectives;
static int numMessages;
static const char *gunName[BT_MAX];
static char *MISSILES_TEXT;
static char *TARGET_TEXT;
static char *NONE_TEXT;
static char *COMBINED_TEXT;
static char *SYSTEM_POWER_TEXT;
static char *LEADER_DIST_TEXT;
static char *TARGET_DIST_TEXT;
static char *OBJECTIVE_DIST_TEXT;
static char *JUMPGATE_DIST_TEXT;
static char *NEW_FIGHTER_TEXT;
static char *SUSPICION_TEXT;
static char *REMAINING_PILOTS_TEXT;
static char *WARNING_TEXT;

void initHud(void)
{
	memset(&hudMessageHead, 0, sizeof(HudMessage));
	hudMessageTail = &hudMessageHead;
	
	gunName[BT_NONE] = "";
	gunName[BT_PARTICLE] = _("Particle Cannon");
	gunName[BT_PLASMA] = _("Plasma Cannon");
	gunName[BT_LASER] = _("Laser Cannon");
	gunName[BT_MAG] = _("Mag Cannon");
	gunName[BT_ROCKET] = _("Rockets");
	gunName[BT_MISSILE] = _("Missiles");
	
	MISSILES_TEXT = _("Missiles (%d)");
	TARGET_TEXT = _("Target: %.2fkm");
	NONE_TEXT = _("(None)");
	COMBINED_TEXT = _("(Combined Guns)");
	SYSTEM_POWER_TEXT = _("System Power : %d%%");
	LEADER_DIST_TEXT = _("%s (Leader)");
	TARGET_DIST_TEXT = _("Target: %.2fkm");
	OBJECTIVE_DIST_TEXT = _("Objective: %.2fkm");
	JUMPGATE_DIST_TEXT = _("Jumpgate: %.2fkm");
	NEW_FIGHTER_TEXT = _("SELECT NEW FIGHTER");
	SUSPICION_TEXT = _("Suspicion");
	REMAINING_PILOTS_TEXT = _("Remaining Pilots: %d");
	WARNING_TEXT = _("WARNING: INCOMING MISSILE!");
	
	targetPointer = getAtlasImage("gfx/hud/targetPointer.png");
	targetCircle = getAtlasImage("gfx/hud/targetCircle.png");
	smallFighter = getAtlasImage("gfx/hud/smallFighter.png");
	arrowLeft = getAtlasImage("gfx/widgets/optionsLeft.png");
	arrowRight = getAtlasImage("gfx/widgets/optionsRight.png");
	armour = getAtlasImage("gfx/hud/armour.png");
	shield = getAtlasImage("gfx/hud/shield.png");
	ecm = getAtlasImage("gfx/hud/ecm.png");
	boost = getAtlasImage("gfx/hud/boost.png");
	nextGun = getAtlasImage("gfx/hud/nextGun.png");
	clockIcon = getAtlasImage("gfx/hud/clock.png");
	objectives = getAtlasImage("gfx/hud/objectives.png");
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
	if (player->alive == ALIVE_ALIVE)
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
		
		drawMissileWarning();
		
		drawSuspicionLevel();
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
	
	setAtlasColor(255, 255, 255, 255);
	
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
	setAtlasColor(255, 255, 255, 255);
	
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
	
	setAtlasColor(255, 255, 255, 255);
	
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
			drawText(30, 70, 14, TA_LEFT, colors.white, NONE_TEXT);
		}
	}
	else
	{
		drawText(30, 70, 14, TA_LEFT, colors.white, COMBINED_TEXT);
	}
	
	drawText(280, 70, 14, TA_RIGHT, colors.white, MISSILES_TEXT, player->missiles);
}

static void drawPlayerTargeter(void)
{
	float angle;
	int x, y;
	
	if (player->target || battle.missionTarget || jumpgateEnabled() || battle.messageSpeaker)
	{
		if (player->target)
		{
			setAtlasColor(255, 0, 0, 255);
		}
		else if (battle.missionTarget)
		{
			setAtlasColor(0, 255, 0, 255);
		}
		else if (battle.messageSpeaker)
		{
			setAtlasColor(255, 255, 255, 255);
		}
		else
		{
			setAtlasColor(255, 255, 0, 255);
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
		
		setAtlasColor(255, 0, 0, 255);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
	
	if (battle.missionTarget)
	{
		angle = getAngle(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		setAtlasColor(0, 255, 0, 255);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
	
	if (jumpgateEnabled())
	{
		angle = getAngle(player->x, player->y, battle.jumpgate->x, battle.jumpgate->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		setAtlasColor(255, 255, 0, 255);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
	
	if (battle.messageSpeaker && battle.messageSpeaker != player)
	{
		angle = getAngle(player->x, player->y, battle.messageSpeaker->x, battle.messageSpeaker->y);
		x = player->x;
		y = player->y;
		
		x += sin(TO_RAIDANS(angle)) * 45;
		y += -cos(TO_RAIDANS(angle)) * 45;
		
		setAtlasColor(255, 255, 255, 255);
		
		blitRotated(targetPointer, x - battle.camera.x, y - battle.camera.y, angle);
	}
}

static void drawNumFighters(void)
{
	/* Allies */
	setAtlasColor(150, 200, 255, 255);
	blit(smallFighter, (app.winWidth / 2) - 200, 15, 0);
	drawText((app.winWidth / 2) - 160, 11, 14, TA_RIGHT, colors.white, battle.numAllies < 1000 ? "(%d)" : "(999+)", battle.numAllies);
	
	/* Enemies */
	setAtlasColor(255, 100, 100, 255);
	blit(smallFighter, (app.winWidth / 2) + 185, 15, 0);
	drawText((app.winWidth / 2) + 160, 11, 14, TA_LEFT, colors.white, !battle.unlimitedEnemies ? "(%d)" : "(999+)", battle.numEnemies);
}

static void drawObjectives(void)
{
	int timeRemaining;
	
	setAtlasColor(255, 255, 255, 255);
	
	if (!game.currentMission->challengeData.isChallenge)
	{
		blit(objectives, (app.winWidth / 2) - 50, 14, 0);
		drawText(app.winWidth / 2, 10, 16, TA_CENTER, colors.white, "%d / %d", battle.numObjectivesComplete, (battle.numObjectivesTotal + battle.numConditions));
		
		if (battle.isEpic && battle.epicLives > 0)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, REMAINING_PILOTS_TEXT, battle.epicLives - 1);
		}
	}
	else
	{
		if (game.currentMission->challengeData.timeLimit)
		{
			timeRemaining = game.currentMission->challengeData.timeLimit - battle.stats[STAT_TIME];
			
			blit(clockIcon, (app.winWidth / 2) - 50, 14, 0);
			drawText(app.winWidth / 2, 10, 16, TA_CENTER, (timeRemaining < 11 * FPS) ? colors.red : colors.white, timeToString(timeRemaining, 0));
		}
		else
		{
			drawText(app.winWidth / 2, 10, 16, TA_CENTER, colors.white, timeToString(battle.stats[STAT_TIME], 0));
			blit(clockIcon, (app.winWidth / 2) - 50, 14, 0);
		}
		
		if (game.currentMission->challengeData.killLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_ENEMIES_KILLED_PLAYER] + battle.stats[STAT_ENEMIES_DISABLED], game.currentMission->challengeData.killLimit);
		}
		else if (game.currentMission->challengeData.itemLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_ITEMS_COLLECTED] + battle.stats[STAT_ITEMS_COLLECTED_PLAYER], game.currentMission->challengeData.itemLimit);
		}
		else if (game.currentMission->challengeData.playerItemLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_ITEMS_COLLECTED_PLAYER], game.currentMission->challengeData.playerItemLimit);
		}
		else if (game.currentMission->challengeData.rescueLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_CIVILIANS_RESCUED], game.currentMission->challengeData.rescueLimit);
		}
		else if (game.currentMission->challengeData.disableLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_ENEMIES_DISABLED], game.currentMission->challengeData.disableLimit);
		}
		else if (game.currentMission->challengeData.surrenderLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_ENEMIES_SURRENDERED], game.currentMission->challengeData.surrenderLimit);
		}
		else if (game.currentMission->challengeData.waypointLimit)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, "%d / %d", battle.stats[STAT_WAYPOINTS_VISITED], game.currentMission->challengeData.waypointLimit);
		}
		else if (player->flags & EF_MUST_DISABLE)
		{
			drawText(app.winWidth / 2, 35, 14, TA_CENTER, colors.white, SYSTEM_POWER_TEXT, player->systemPower);
		}
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
	
	if (player->target)
	{
		if (player->target->flags & EF_AI_LEADER && player->target->speed > 0)
		{
			drawText(app.winWidth - 15, y, 18, TA_RIGHT, colors.red, LEADER_DIST_TEXT, player->target->name);
		}
		else
		{
			drawText(app.winWidth - 15, y, 18, TA_RIGHT, colors.red, player->target->name);
		}
		
		y += 30;
		
		distance = distanceToKM(player->x, player->y, player->target->x, player->target->y);
		
		drawText(app.winWidth - 15, y, 14, TA_RIGHT, colors.red, TARGET_DIST_TEXT, distance);
		
		y += 25;
	}
	
	if (battle.missionTarget)
	{
		distance = distanceToKM(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
		
		drawText(app.winWidth - 15, y, 14, TA_RIGHT, colors.green, OBJECTIVE_DIST_TEXT, distance);
		
		y += 25;
	}
	
	if (jumpgateEnabled())
	{
		distance = distanceToKM(player->x, player->y, battle.jumpgate->x, battle.jumpgate->y);
		
		drawText(app.winWidth - 15, y, 14, TA_RIGHT, colors.yellow, JUMPGATE_DIST_TEXT, distance);
		
		y += 25;
	}
	
	if (battle.messageSpeaker)
	{
		distance = distanceToKM(player->x, player->y, battle.messageSpeaker->x, battle.messageSpeaker->y);
		
		drawText(app.winWidth - 15, y, 14, TA_RIGHT, colors.white, "%s: %.2fkm", battle.messageSpeaker->name, distance);
		
		y += 25;
	}
}

static void drawHudMessages(void)
{
	HudMessage *hudMessage;
	int y = app.winHeight - 25;
	
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
	
	setAtlasColor(0, 200, 255, 255);
	
	blit(targetCircle, player->x - battle.camera.x, player->y - battle.camera.y, 1);
	
	drawText(app.winWidth / 2, 500, 28, TA_CENTER, colors.white, NEW_FIGHTER_TEXT);
	
	if (player->health > 0)
	{
		drawText(app.winWidth / 2, 540, 20, TA_CENTER, colors.white, "%s (%d%% / %d%%)", player->defName, getPercent(player->health, player->maxHealth), getPercent(player->shield, player->maxShield));
	}
	
	setAtlasColor(255, 255, 255, 255);
	
	blit(arrowLeft, (app.winWidth / 2) - 200, 520, 1);
	blit(arrowRight, (app.winWidth / 2) + 200, 520, 1);
}

static void drawSuspicionLevel(void)
{
	SDL_Rect r;
	
	if (battle.hasSuspicionLevel && !battle.incomingMissile)
	{
		battle.suspicionLevel = MIN(battle.suspicionLevel, MAX_SUSPICION_LEVEL);
		
		drawText((app.winWidth / 2) - 150, app.winHeight - 60, 18, TA_RIGHT, colors.white, SUSPICION_TEXT);
		
		r.x = (app.winWidth / 2) - 140;
		r.y = app.winHeight - 58;
		r.w = 400;
		r.h = 20;
		
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		SDL_RenderFillRect(app.renderer, &r);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		
		SDL_SetRenderDrawColor(app.renderer, 192, 192, 192, 255);
		SDL_RenderDrawRect(app.renderer, &r);
		
		r.x += 2;
		r.y += 2;
		r.w -= 4;
		r.h -= 4;
		
		r.w = MAX((r.w / MAX_SUSPICION_LEVEL) * battle.suspicionLevel, 0);
		
		if (battle.suspicionLevel < (MAX_SUSPICION_LEVEL * 0.5))
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
		}
		else if (battle.suspicionLevel < (MAX_SUSPICION_LEVEL * 0.75))
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 128, 0, 255);
		}
		else
		{
			SDL_SetRenderDrawColor(app.renderer, 255, 0, 0, 255);
		}
		
		SDL_RenderFillRect(app.renderer, &r);
		
		drawText(r.x + r.w + 7, app.winHeight - 57, 12, TA_LEFT, colors.white, "%d%%", (battle.suspicionLevel > 0) ? getPercent(battle.suspicionLevel, MAX_SUSPICION_LEVEL) : 0);
	}
}

static void drawMissileWarning(void)
{
	if (battle.incomingMissile && battle.stats[STAT_TIME] % FPS < 40)
	{
		drawText(app.winWidth / 2, app.winHeight - 60, 18, TA_CENTER, colors.red, WARNING_TEXT);
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
