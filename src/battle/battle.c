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
	battle.entityTail = &battle.entityHead;
	battle.effectTail = &battle.effectHead;
	battle.objectiveTail = &battle.objectiveHead;
	battle.triggerTail = &battle.triggerHead;
	
	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	initStars();
	
	initBackground();
	
	initHud();
	
	initMissionInfo();
	
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
	if (battle.status == MS_IN_PROGRESS || battle.status == MS_COMPLETE || battle.status == MS_FAILED)
	{
		handleKeyboard();
		
		if (show == SHOW_BATTLE)
		{
			if (!battle.epic || (battle.epic && !battle.playerSelect))
			{
				doBattle();
			}
			else if (battle.epic && battle.playerSelect)
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
	
	doHud();
	
	doObjectives();
	
	doStars(ssx, ssy);
	
	doBullets();
	
	doEntities();
	
	doEffects();
	
	doPlayer();
	
	if (battle.status != MS_IN_PROGRESS)
	{
		battle.missionFinishedTimer--;
		
		if (battle.missionFinishedTimer == 0)
		{
			if (battle.status == MS_COMPLETE)
			{
				selectWidget("continue", "battleWon");
			}
			else
			{
				selectWidget("retry", "battleLost");
			}
		}
	}
	
	battle.stats[STAT_TIME]++;
	if (battle.stats[STAT_TIME] % FPS == 0)
	{
		checkTrigger("TIME", TRIGGER_TIME);
	}
}

static void draw(void)
{
	prepareScene();
	
	if (player != NULL)
	{
		battle.camera.x = player->x - (SCREEN_WIDTH / 2);
		battle.camera.y = player->y - (SCREEN_HEIGHT / 2);
	}
	
	drawBackground(battle.background);
	
	drawStars();
	
	blit(battle.planetTexture, battle.planet.x - battle.camera.x, battle.planet.y - battle.camera.y, 1);
	
	drawBullets();
	
	drawEntities();
	
	drawEffects();
	
	drawHud();
	
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
	
	presentScene();
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
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
		
	drawWidgets("inBattle");
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
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
		
		memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
		
		playSound(SND_GUI_CLOSE);
	}
	
	if (app.keyboard[SDL_SCANCODE_TAB])
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
	
	memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
}

static void continueGame(void)
{
	postBattle();
	
	destroyBattle();
	
	resetHud();
	
	initGalacticMap();
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
	
	resetHud();
	
	initBattle();
	
	loadMission(game.currentMission->filename);
}

static void quitBattle(void)
{
	postBattle();
	
	destroyBattle();
	
	resetHud();
	
	initGalacticMap();
}

static void postBattle(void)
{
	int i;
	
	/* we don't want to count the time when adding up stats */
	for (i = 0 ; i < STAT_TIME ; i++)
	{
		game.stats[i] += battle.stats[i];
	}
	
	if (game.currentMission && !game.currentMission->completed)
	{
		game.currentMission->completed = (battle.status == MS_COMPLETE || !battle.numObjectivesTotal);
	}
	
}

void destroyBattle(void)
{
	Entity *ent;
	Bullet *b;
	Effect *e;
	Objective *o;
	Trigger *t;
	
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
	
	while (battle.triggerHead.next)
	{
		t = battle.triggerHead.next;
		battle.triggerHead.next = t->next;
		free(t);
	}
	battle.triggerTail = &battle.triggerHead;
}
