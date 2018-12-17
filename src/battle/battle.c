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

#include "battle.h"

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void postBattle(void);
void destroyBattle(void);
static void doBattle(void);
static void optQuitBattle(void);
static void quitBattle(void);
static void drawMenu(void);
static void continueGame(void);
static void resume(void);
static void restart(void);
static void retry(void);
static void start(void);
static void options(void);
static void returnFromOptions(void);
static void checkSuspicionLevel(void);
static void doTorelliFireStorm(void);
static void endCampaign(void);

static int show;
static float ssx, ssy;

void initBattle(void)
{
	memset(&battle, 0, sizeof(Battle));
	battle.bulletTail = &battle.bulletHead;
	battle.debrisTail = &battle.debrisHead;
	battle.entityTail = &battle.entityHead;
	battle.effectTail = &battle.effectHead;
	battle.objectiveTail = &battle.objectiveHead;
	battle.locationTail = &battle.locationHead;
	battle.spawnerTail = &battle.spawnerHead;

	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	battle.hasThreats = 1;

	initQuadtree(&battle.quadtree);

	initBullets();

	initEntities();

	initStars();

	initEffects();

	initHud();

	initRadar();

	initMessageBox();

	initDebris();

	resetWaypoints();

	show = SHOW_BATTLE;

	getWidget("ok", "startBattle")->action = start;

	getWidget("resume", "inBattle")->action = resume;
	getWidget("options", "inBattle")->action = options;
	getWidget("restart", "inBattle")->action = restart;
	getWidget("quit", "inBattle")->action = optQuitBattle;

	getWidget("continue", "battleWon")->action = continueGame;
	getWidget("retry", "battleWon")->action = retry;

	getWidget("retry", "battleLost")->action = retry;
	getWidget("quit", "battleLost")->action = quitBattle;

	selectWidget("ok", "startBattle");
	
	SDL_SetWindowGrab(app.window, 1);
}

static void logic(void)
{
	if (battle.status == MS_IN_PROGRESS || battle.status == MS_COMPLETE || battle.status == MS_FAILED || battle.status == MS_TIME_UP)
	{
		handleKeyboard();

		if (show == SHOW_BATTLE)
		{
			if (!battle.isEpic || (battle.isEpic && !battle.playerSelect))
			{
				doBattle();
			}
			else if (battle.isEpic && battle.playerSelect)
			{
				doPlayerSelect();
			}
		}
		
		app.doTrophyAlerts = (battle.status != MS_IN_PROGRESS && battle.missionFinishedTimer <= -FPS * 2);
		
		if (battle.campaignFinished)
		{
			endCampaign();
		}
	}

	doWidgets();
}

static void doBattle(void)
{
	if (player->alive == ALIVE_ALIVE)
	{
		ssx = player->dx;
		ssy = player->dy;
	}
	else
	{
		ssx = ssy = 0;
	}

	scrollBackground(-ssx * 0.1, -ssy * 0.1);

	battle.planet.x -= ssx * 0.1;
	battle.planet.y -= ssy * 0.1;

	doObjectives();

	doChallenges();

	doHud();

	doStars(ssx, ssy);

	doBullets();

	doEntities();

	doEffects();

	doDebris();

	doPlayer();
	
	checkSuspicionLevel();
	
	doTorelliFireStorm();

	if (player->alive == ALIVE_ALIVE)
	{
		doSpawners();

		doLocations();

		doMessageBox();
		
		if (battle.status == MS_IN_PROGRESS || battle.status == MS_COMPLETE)
		{
			doScript();
		}

		if (battle.status == MS_IN_PROGRESS)
		{
			if (battle.stats[STAT_TIME]++ % FPS == 0)
			{
				runScriptFunction("TIME %d", battle.stats[STAT_TIME] / FPS);
				
				if (game.currentMission->challengeData.timeLimit && game.currentMission->challengeData.timeLimit - battle.stats[STAT_TIME] < 11 * FPS)
				{
					playSound(SND_TIME_WARNING);
				}
			}
		}
	}

	if (battle.status != MS_IN_PROGRESS)
	{
		battle.missionFinishedTimer--;
		
		if (battle.unwinnable && battle.missionFinishedTimer <= -FPS * 6)
		{
			battle.status = MS_COMPLETE;
			
			postBattle();

			destroyBattle();

			initGalacticMap();
		}
	}
}

static void draw(void)
{
	if (player->alive == ALIVE_ALIVE)
	{
		battle.camera.x = player->x - (app.winWidth / 2);
		battle.camera.y = player->y - (app.winHeight / 2);
	}

	drawBackground(battle.background);
	
	setAtlasColor(255, 255, 255, 255);

	blitScaled(battle.planetTexture, battle.planet.x, battle.planet.y, battle.planetWidth, battle.planetHeight, 0);
	
	if (battle.destroyTorelli)
	{
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(battle.fireStormTexture->texture, battle.torelliFireStormAlpha);
		blitScaled(battle.fireStormTexture, battle.planet.x, battle.planet.y, battle.planetWidth, battle.planetHeight, 0);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	}

	drawStars();

	drawEntities();

	drawBullets();

	drawDebris();

	drawEffects();

	if (dev.debug)
	{
		drawLocations();
	}

	drawHud();

	if (player->alive == ALIVE_ALIVE)
	{
		drawMessageBox();
	}

	drawMissionInfo();

	switch (show)
	{
		case SHOW_MENU:
			drawMenu();
			break;

		case SHOW_OPTIONS:
			drawOptions();
			break;
	}
}

static void drawMenu(void)
{
	SDL_Rect r;
	
	if (app.modalDialog.type == MD_NONE)
	{
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
		SDL_RenderFillRect(app.renderer, NULL);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		
		SDL_SetRenderTarget(app.renderer, app.uiBuffer);

		r.w = 400;
		r.h = 400;
		r.x = (UI_WIDTH / 2) - r.w / 2;
		r.y = (UI_HEIGHT / 2) - r.h / 2;

		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(app.renderer, &r);
		SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
		SDL_RenderDrawRect(app.renderer, &r);

		drawWidgets("inBattle");
		
		SDL_SetRenderTarget(app.renderer, app.backBuffer);
	}
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE] && !app.awaitingWidgetInput && battle.status == MS_IN_PROGRESS)
	{
		switch (show)
		{
			case SHOW_BATTLE:
			case SHOW_OPTIONS:
				selectWidget("resume", "inBattle");
				show = SHOW_MENU;
				SDL_SetWindowGrab(app.window, 0);
				break;

			case SHOW_MENU:
				show = SHOW_BATTLE;
				SDL_SetWindowGrab(app.window, 1);
				break;

			case SHOW_OBJECTIVES:
				show = SHOW_BATTLE;
				SDL_SetWindowGrab(app.window, 1);
				break;
		}

		clearInput();

		playSound(SND_GUI_CLOSE);
	}

	if (battle.status == MS_IN_PROGRESS && app.keyboard[SDL_SCANCODE_TAB])
	{
		battle.status = MS_PAUSED;
		
		selectWidget("ok", "startBattle");
		
		SDL_SetWindowGrab(app.window, 0);
	}
}

static void start(void)
{
	battle.status = MS_IN_PROGRESS;
	
	SDL_SetWindowGrab(app.window, 1);
}

static void resume(void)
{
	show = SHOW_BATTLE;
	
	SDL_SetWindowGrab(app.window, 1);

	clearInput();
}

static void continueGame(void)
{
	postBattle();

	destroyBattle();

	if (!game.currentMission->challengeData.isChallenge)
	{
		initGalacticMap();
	}
	else
	{
		initChallengeHome();
	}
}

static void options(void)
{
	show = SHOW_OPTIONS;

	initOptions(returnFromOptions);
}

static void returnFromOptions(void)
{
	show = SHOW_MENU;

	selectWidget("resume", "inBattle");
}

static void ignoreRestartQuit(void)
{
	app.modalDialog.type = MD_NONE;
}

static void restart(void)
{
	showOKCancelDialog(&retry, &ignoreRestartQuit, _("Are you sure you want to restart? You will lose your current progress."));
}

static void retry(void)
{
	app.modalDialog.type = MD_NONE;
	
	postBattle();

	destroyBattle();

	initBattle();

	loadMission(game.currentMission->filename);
}

static void optQuitBattle(void)
{
	showOKCancelDialog(&quitBattle, &ignoreRestartQuit, _("Are you sure you want to quit? You will lose your current progress."));
}

static void quitBattle(void)
{
	app.modalDialog.type = MD_NONE;
	
	postBattle();

	destroyBattle();

	if (!game.currentMission->challengeData.isChallenge)
	{
		initGalacticMap();
	}
	else
	{
		initChallengeHome();
	}
}

static void postBattle(void)
{
	int i;

	for (i = 0 ; i < STAT_MAX ; i++)
	{
		if (i != STAT_TIME && i != STAT_EPIC_KILL_STREAK)
		{
			game.stats[i] += battle.stats[i];
		}
	}
	
	game.stats[STAT_EPIC_KILL_STREAK] = MAX(game.stats[STAT_EPIC_KILL_STREAK], battle.stats[STAT_EPIC_KILL_STREAK]);

	updateAccuracyStats(game.stats);
	
	game.currentMission->completed = (game.currentMission->completed || battle.status == MS_COMPLETE || !battle.numObjectivesTotal);
	
	app.saveGame = 1;
}

static void checkSuspicionLevel(void)
{
	if (battle.hasSuspicionLevel && battle.suspicionLevel >= MAX_SUSPICION_LEVEL)
	{
		cancelScript();
		
		resetMessageBox();
		
		runScriptFunction("MAX_SUSPICION_LEVEL");
		
		battle.hasSuspicionLevel = 0;
	}
}

static void doTorelliFireStorm(void)
{
	if (battle.destroyTorelli)
	{
		battle.torelliFireStormAlpha = MIN(battle.torelliFireStormAlpha + 0.25, 255);
	}
}

static void endCampaign(void)
{
	awardTrophy("CAMPAIGN_COMPLETE");
	
	postBattle();

	destroyBattle();
	
	initCredits();
}

void destroyBattle(void)
{
	Entity *ent;
	Bullet *b;
	Debris *d;
	Effect *e;
	Objective *o;
	Location *l;
	Spawner *s;

	while (battle.entityHead.next)
	{
		ent = battle.entityHead.next;
		battle.entityHead.next = ent->next;
		free(ent);
	}
	battle.entityTail = &battle.entityHead;

	while (battle.bulletHead.next)
	{
		b = battle.bulletHead.next;
		battle.bulletHead.next = b->next;
		free(b);
	}
	battle.bulletTail = &battle.bulletHead;

	while (battle.debrisHead.next)
	{
		d = battle.debrisHead.next;
		battle.debrisHead.next = d->next;
		free(d);
	}
	battle.debrisTail = &battle.debrisHead;

	while (battle.effectHead.next)
	{
		e = battle.effectHead.next;
		battle.effectHead.next = e->next;
		free(e);
	}
	battle.effectTail = &battle.effectHead;

	while (battle.objectiveHead.next)
	{
		o = battle.objectiveHead.next;
		battle.objectiveHead.next = o->next;
		free(o);
	}
	battle.objectiveTail = &battle.objectiveHead;

	while (battle.locationHead.next)
	{
		l = battle.locationHead.next;
		battle.locationHead.next = l->next;
		free(l);
	}
	battle.locationTail = &battle.locationHead;

	while (battle.spawnerHead.next)
	{
		s = battle.spawnerHead.next;
		battle.spawnerHead.next = s->next;
		free(s);
	}
	battle.spawnerTail = &battle.spawnerHead;

	cJSON_Delete(battle.missionJSON);

	resetHud();

	resetMessageBox();

	destroyEntities();

	destroyScript();

	destroyQuadtree();

	destroyDebris();

	destroyBullets();

	destroyEffects();
	
	memset(&battle, 0, sizeof(Battle));
	battle.bulletTail = &battle.bulletHead;
	battle.debrisTail = &battle.debrisHead;
	battle.entityTail = &battle.entityHead;
	battle.effectTail = &battle.effectHead;
	battle.objectiveTail = &battle.objectiveHead;
	battle.locationTail = &battle.locationHead;
	battle.spawnerTail = &battle.spawnerHead;
}
