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

#include "battle.h"

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void postBattle(void);
void destroyBattle(void);
static void doBattle(void);
static void quitBattle(void);
static void drawMenu(void);
static void continueGame(void);
static void resume(void);
static void retry(void);
static void start(void);
static void options(void);
static void returnFromOptions(void);

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

	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);

	initQuadtree(&battle.quadtree);

	initBullets();

	initEntities();

	initStars();

	initBackground();

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
	getWidget("restart", "inBattle")->action = retry;
	getWidget("quit", "inBattle")->action = quitBattle;

	getWidget("continue", "battleWon")->action = continueGame;
	getWidget("retry", "battleWon")->action = retry;

	getWidget("retry", "battleLost")->action = retry;
	getWidget("quit", "battleLost")->action = quitBattle;

	selectWidget("ok", "startBattle");
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
	}

	doWidgets();
}

static void doBattle(void)
{
	if (player != NULL)
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

	if (player != NULL)
	{
		doLocations();

		doMessageBox();

		if (battle.status == MS_IN_PROGRESS)
		{
			doScript();

			battle.stats[STAT_TIME]++;

			if (battle.stats[STAT_TIME] % FPS == 0)
			{
				runScriptFunction("TIME %d", battle.stats[STAT_TIME] / 60);
			}
		}
	}

	if (battle.status != MS_IN_PROGRESS)
	{
		battle.missionFinishedTimer--;
	}

	if (battle.unwinnable && battle.missionFinishedTimer <= -FPS * 6)
	{
		postBattle();

		destroyBattle();

		initGalacticMap();
	}
}

static void draw(void)
{
	if (player != NULL)
	{
		battle.camera.x = player->x - (SCREEN_WIDTH / 2);
		battle.camera.y = player->y - (SCREEN_HEIGHT / 2);
	}

	drawBackground(battle.background);

	blitScaled(battle.planetTexture, battle.planet.x, battle.planet.y, battle.planetWidth, battle.planetHeight);

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

	if (player != NULL)
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

	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);

	r.w = 400;
	r.h = 400;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;

	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(app.renderer, &r);

	drawWidgets("inBattle");
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE] && !app.awaitingWidgetInput)
	{
		switch (show)
		{
			case SHOW_BATTLE:
			case SHOW_OPTIONS:
				selectWidget("resume", "inBattle");
				show = SHOW_MENU;
				break;

			case SHOW_MENU:
				show = SHOW_BATTLE;
				break;

			case SHOW_OBJECTIVES:
				show = SHOW_BATTLE;
				break;
		}

		clearInput();

		playSound(SND_GUI_CLOSE);
	}

	if (battle.status == MS_IN_PROGRESS && app.keyboard[SDL_SCANCODE_TAB])
	{
		battle.status = MS_PAUSED;
	}
}

static void start(void)
{
	battle.status = MS_IN_PROGRESS;
}

static void resume(void)
{
	show = SHOW_BATTLE;

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

static void retry(void)
{
	postBattle();

	destroyBattle();

	initBattle();

	loadMission(game.currentMission->filename);
}

static void quitBattle(void)
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

	updateAccuracyStats(game.stats);

	if (!game.currentMission->challengeData.isChallenge)
	{
		if (game.currentMission && !game.currentMission->completed)
		{
			game.currentMission->completed = (battle.status == MS_COMPLETE || !battle.numObjectivesTotal);
		}
	}
}

void destroyBattle(void)
{
	Entity *ent;
	Bullet *b;
	Debris *d;
	Effect *e;
	Objective *o;
	Location *l;

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

	cJSON_Delete(battle.missionJSON);

	resetHud();

	resetMessageBox();

	destroyEntities();

	destroyScript();

	destroyQuadtree();

	destroyDebris();

	destroyBullets();

	destroyEffects();
}
