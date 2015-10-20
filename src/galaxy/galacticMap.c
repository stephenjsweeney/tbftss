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

#include "galacticMap.h"

static void logic(void);
static void draw(void);
static void handleKeyboard(void);
static void drawStarSystemDetail(void);
static void prevMission(void);
static void nextMission(void);
static void selectStarSystem(void);
static void drawGalaxy(void);
static void handleGalaxyKB(void);
static void handleSelectedSystemKB(void);
static void centerOnSelectedStarSystem(void);
static void doStarSystems(void);
void destroyGalacticMap(void);
static void drawPulses(void);
static void doPulses(void);
static void addPulses(void);
static void drawMenu(void);
static void resume(void);
static void stats(void);
static void options(void);
static void statsOK(void);
static void quit(void);
static void returnFromOptions(void);

static StarSystem *selectedStarSystem;
static Mission *selectedMission = {0};
static int missionListStart, selectedMissionIndex;
static SDL_Texture *background;
static SDL_Texture *starSystemTexture;
static SDL_Point camera;
static int viewingSystem;
static Pulse pulseHead = {0};
static Pulse *pulseTail;
static int pulseTimer;
static float ssx, ssy;
static int show;
static int completedMissions, totalMissions;
static int completedChallenges, totalChallenges;

void initGalacticMap(void)
{
	startSectionTransition();
	
	stopMusic();
	
	app.delegate.logic = &logic;
	app.delegate.draw = &draw;
	memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	
	background = getTexture("gfx/backgrounds/background02.jpg");
	
	starSystemTexture = getTexture("gfx/galaxy/starSystem.png");
	
	if (!selectedStarSystem)
	{
		selectedStarSystem = game.starSystemHead.next;
	}
	
	updateStarSystemDescriptions();
	
	centerOnSelectedStarSystem();
	
	saveGame();
	
	viewingSystem = 0;
	
	pulseTimer = 0;
	
	show = SHOW_GALAXY;
	
	/* clear the pulses */
	destroyGalacticMap();
	
	initBackground();
	
	getWidget("resume", "galacticMap")->action = resume;
	getWidget("stats", "galacticMap")->action = stats;
	getWidget("options", "galacticMap")->action = options;
	getWidget("quit", "galacticMap")->action = quit;
	
	getWidget("ok", "stats")->action = statsOK;
	
	endSectionTransition();
	
	playMusic("music/Pressure.ogg");
}

static void logic(void)
{
	handleKeyboard();
	
	doStarSystems();
	
	scrollBackground(-ssx, -ssy);
	
	doPulses();
	
	doStars(ssx, ssy);
	
	if (pulseTimer % FPS == 0)
	{
		addPulses();
	}
	
	pulseTimer++;
	pulseTimer %= (FPS * 60);
	
	doWidgets();
}

static void doStarSystems(void)
{
	StarSystem *starSystem;
	int cx, cy;
	
	completedMissions = totalMissions = completedChallenges = totalChallenges = 0;
	
	cx = (camera.x + SCREEN_WIDTH / 2) - 32;
	cy = (camera.y + SCREEN_HEIGHT / 2) - 32;
	
	selectedStarSystem = NULL;
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		completedMissions += starSystem->completedMissions;
		totalMissions += starSystem->totalMissions;
		completedChallenges += starSystem->completedChallenges;
		totalChallenges += starSystem->totalChallenges;
		
		if (starSystem->totalMissions > 0 && collision(cx, cy, 64, 64, starSystem->x, starSystem->y, 4, 4))
		{
			if (selectedStarSystem != starSystem)
			{
				selectedStarSystem = starSystem;
			}
		}
	}
}

static void addPulses(void)
{
	Pulse *pulse;
	StarSystem *starSystem;
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		if (starSystem->completedMissions < starSystem->totalMissions)
		{
			pulse = malloc(sizeof(Pulse));
			memset(pulse, 0, sizeof(Pulse));
			
			pulse->x = starSystem->x;
			pulse->y = starSystem->y;
			pulse->life = 255;
			pulse->r = 255;
			
			pulseTail->next = pulse;
			pulseTail = pulse;
		}
		else if (starSystem->completedChallenges < starSystem->totalChallenges && pulseTimer % (FPS * 2) == 0)
		{
			pulse = malloc(sizeof(Pulse));
			memset(pulse, 0, sizeof(Pulse));
			
			pulse->x = starSystem->x;
			pulse->y = starSystem->y;
			pulse->life = 255;
			pulse->r = pulse->g = 255;
			
			pulseTail->next = pulse;
			pulseTail = pulse;
		}
	}
}

static void doPulses(void)
{
	Pulse *pulse, *prev;
	
	prev = &pulseHead;
	
	for (pulse = pulseHead.next ; pulse != NULL ; pulse = pulse->next)
	{
		pulse->size += 0.5;
		pulse->life--;
		
		if (pulse->life <= 0)
		{
			if (pulse == pulseTail)
			{
				pulseTail = prev;
			}
			
			prev->next = pulse->next;
			free(pulse);
			pulse = prev;
		}
		
		prev = pulse;
	}
}

static void draw(void)
{
	prepareScene();
	
	drawBackground(background);
	
	drawStars();
	
	drawGalaxy();
	
	drawPulses();
	
	if (viewingSystem)
	{
		drawStarSystemDetail();
	}
	
	switch (show)
	{
		case SHOW_GALAXY:
			break;
			
		case SHOW_MENU:
			drawMenu();
			break;
			
		case SHOW_STATS:
			drawStats();
			break;
			
		case SHOW_OPTIONS:
			drawOptions();
			break;
	}
	
	presentScene();
}

static void drawPulses(void)
{
	Pulse *pulse;
	
	for (pulse = pulseHead.next ; pulse != NULL ; pulse = pulse->next)
	{
		drawCircle(pulse->x - camera.x, pulse->y - camera.y, pulse->size, pulse->r, pulse->g, pulse->b, pulse->life);
	}
}

static void centerOnSelectedStarSystem(void)
{
	camera.x = selectedStarSystem->x;
	camera.x -= SCREEN_WIDTH / 2;
	
	camera.y = selectedStarSystem->y;
	camera.y -= SCREEN_HEIGHT / 2;
}

static void drawGalaxy(void)
{
	SDL_Rect r;
	StarSystem *starSystem;
	SDL_Color color;
	
	r.w = r.h = 64;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	if (selectedStarSystem)
	{
		SDL_SetRenderDrawColor(app.renderer, 64, 100, 128, 255);
		SDL_RenderFillRect(app.renderer, &r);
	}
	
	SDL_SetRenderDrawColor(app.renderer, 128, 200, 255, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		blit(starSystemTexture, starSystem->x - camera.x, starSystem->y - camera.y, 1);
		
		switch (starSystem->side)
		{
			case SIDE_CSN:
				color = colors.cyan;
				break;
				
			case SIDE_UNF:
				color = colors.white;
				break;
				
			case SIDE_INF:
				color = colors.red;
				break;
		}
		
		drawText(starSystem->x - camera.x, starSystem->y - camera.y + 12, 14, TA_CENTER, color, starSystem->name);
	}
	
	if (!viewingSystem && selectedStarSystem != NULL)
	{
		r.x = 0;
		r.y = SCREEN_HEIGHT - 35;
		r.w = SCREEN_WIDTH;
		r.h = 35;
		
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 200);
		SDL_RenderFillRect(app.renderer, &r);
		SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
		
		drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30, 18, TA_CENTER, colors.white, selectedStarSystem->description);
	}
	
	r.x = 0;
	r.y = 0;
	r.w = SCREEN_WIDTH;
	r.h = 35;
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 200);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	drawText((SCREEN_WIDTH / 2) - 50, 5, 18, TA_RIGHT, colors.white, "Missions %d / %d", completedMissions, totalMissions);
	drawText((SCREEN_WIDTH / 2) + 50, 5, 18, TA_LEFT, colors.white, "Challenges %d / %d", completedChallenges, totalChallenges);
}

static void selectStarSystem(void)
{
	Mission *mission, *prev;
	
	selectedMission = selectedStarSystem->missionHead.next;
	
	selectedStarSystem->completedMissions = selectedStarSystem->totalMissions = 0;
	
	prev = &selectedStarSystem->missionHead;
	
	for (mission = selectedStarSystem->missionHead.next ; mission != NULL ; mission = mission->next)
	{
		selectedStarSystem->totalMissions++;
		
		if (mission->completed)
		{
			selectedStarSystem->completedMissions++;
		}
		
		mission->available = prev->completed;
		
		prev = mission;
	}
	
	missionListStart = 0;
	selectedMissionIndex = 0;
	
	viewingSystem = selectedStarSystem->totalMissions > 0;
}

static void drawStarSystemDetail(void)
{
	int y, i;
	Mission *mission;
	Challenge *challenge;
	SDL_Rect r;
	
	r.w = 900;
	r.h = 600;
	r.x = (SCREEN_WIDTH / 2) - (r.w / 2);
	r.y = (SCREEN_HEIGHT / 2) - (r.h / 2);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 225);
	SDL_RenderFillRect(app.renderer, &r);
	
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 200);
	SDL_RenderDrawRect(app.renderer, &r);
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	y = 70;
	
	drawText(SCREEN_WIDTH / 2, y, 28, TA_CENTER, colors.cyan, "%s (%d / %d)", selectedStarSystem->name, selectedStarSystem->completedMissions, selectedStarSystem->totalMissions);
	
	SDL_RenderDrawLine(app.renderer, r.x, 120, r.x + r.w, 120);
	
	SDL_RenderDrawLine(app.renderer, 515, 120, 515, 660);
	
	y += 80;
	
	r.x = 200;
	r.w = 300;
	r.h = 40;
	
	i = 0;
	
	for (mission = selectedStarSystem->missionHead.next ; mission != NULL ; mission = mission->next)
	{
		if (i == selectedMissionIndex)
		{
			selectedMission = mission;
		}
		
		if (i >= missionListStart && i < missionListStart + 10)
		{
			if (mission == selectedMission)
			{
				r.y = y - 2;
				
				SDL_SetRenderDrawColor(app.renderer, 32, 64, 128, 255);
				SDL_RenderFillRect(app.renderer, &r);
				
				SDL_SetRenderDrawColor(app.renderer, 64, 96, 196, 255);
				SDL_RenderDrawRect(app.renderer, &r);
			}
			
			if (mission->available)
			{
				drawText(210, y, 24, TA_LEFT, mission->completed ? colors.white : colors.yellow, mission->name);
			}
			else
			{
				drawText(210, y, 24, TA_LEFT, colors.darkGrey, "[LOCKED]");
			}
			
			y += 50;
		}
		
		i++;
	}
	
	if (selectedMission->available)
	{
		drawText(525, 135, 18, TA_LEFT, colors.lightGrey, "Pilot: %s", selectedMission->pilot);
		drawText(525, 160, 18, TA_LEFT, colors.lightGrey, "Craft: %s", selectedMission->craft);
		drawText(525, 185, 18, TA_LEFT, colors.lightGrey, "Squadron: %s", selectedMission->squadron);
		
		limitTextWidth(500);
		drawText(525, 230, 22, TA_LEFT, colors.white, selectedMission->description);
		limitTextWidth(0);
	}
	else
	{
		drawText(525, 135, 18, TA_LEFT, colors.lightGrey, "Pilot: -");
		drawText(525, 160, 18, TA_LEFT, colors.lightGrey, "Craft: -");
		drawText(525, 185, 18, TA_LEFT, colors.lightGrey, "Squadron: -");
		
		limitTextWidth(500);
		drawText(525, 230, 22, TA_LEFT, colors.darkGrey, "You cannot play this mission yet. Complete the previous mission to unlock.");
		limitTextWidth(0);
	}
	
	if (selectedMission && selectedMission->available && selectedMission->challengeHead.next)
	{
		y = SCREEN_HEIGHT - 100;
		
		for (challenge = selectedMission->challengeHead.next ; challenge != NULL ; challenge = challenge->next)
		{
			y -= 25;
		}
		
		drawText(525, y, 22, TA_LEFT, colors.yellow, "Challenges");
		
		y += 30;
		
		for (challenge = selectedMission->challengeHead.next ; challenge != NULL ; challenge = challenge->next)
		{
			drawText(525, y, 18, TA_LEFT, challenge->passed ? colors.green : colors.lightGrey, "%s%s", getChallengeDescription(challenge), challenge->passed ? " - COMPLETE" : "");
			
			y += 25;
		}
	}
}

static void handleKeyboard(void)
{
	if (show == SHOW_GALAXY)
	{
		if (viewingSystem)
		{
			handleSelectedSystemKB();
		}
		else
		{
			handleGalaxyKB();
		}
	}
	else if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		switch (show)
		{
			case SHOW_MENU:
				show = SHOW_GALAXY;
				memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
				break;
				
			case SHOW_OPTIONS:
			case SHOW_STATS:
				show = SHOW_MENU;
				selectWidget("resume", "galacticMap");
				memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
				break;
		}
		
		playSound(SND_GUI_CLOSE);
	}
}

static void handleGalaxyKB(void)
{
	int lastX, lastY;
	
	lastX = camera.x;
	lastY = camera.y;
	
	ssx = ssy = 0;
	
	if (app.keyboard[SDL_SCANCODE_LEFT])
	{
		ssx = -1;
		camera.x -= CAMERA_SPEED;
	}
	
	if (app.keyboard[SDL_SCANCODE_RIGHT])
	{
		ssx = 1;
		camera.x += CAMERA_SPEED;
	}
	
	if (app.keyboard[SDL_SCANCODE_UP])
	{
		ssy = -1;
		camera.y -= CAMERA_SPEED;
	}
	
	if (app.keyboard[SDL_SCANCODE_DOWN])
	{
		ssy = 1;
		camera.y += CAMERA_SPEED;
	}
	
	if (app.keyboard[SDL_SCANCODE_RETURN] && selectedStarSystem)
	{
		selectStarSystem();
		memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	}
	
	camera.x = MAX(-800, MIN(camera.x, 2464));
	camera.y = MAX(-475, MIN(camera.y, 1235));
	
	if (lastX == camera.x)
	{
		ssx = 0;
	}
	
	if (lastY == camera.y)
	{
		ssy = 0;
	}
	
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		selectWidget("resume", "galacticMap");
		show = SHOW_MENU;
		memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
		
		playSound(SND_GUI_CLOSE);
	}
}

static void handleSelectedSystemKB(void)
{
	if (app.keyboard[SDL_SCANCODE_UP])
	{
		prevMission();
	}
	
	if (app.keyboard[SDL_SCANCODE_DOWN])
	{
		nextMission();
	}
	
	if (app.keyboard[SDL_SCANCODE_RETURN])
	{
		if (selectedMission->available)
		{
			playSound(SND_GUI_SELECT);
			
			initBattle();
			game.currentMission = selectedMission;
			loadMission(selectedMission->filename);
		}
		else
		{
			playSound(SND_GUI_DENIED);
		}
	}
	
	if (app.keyboard[SDL_SCANCODE_ESCAPE])
	{
		viewingSystem = 0;
		
		playSound(SND_GUI_CLOSE);
	}
	
	memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
}

static void prevMission(void)
{
	selectedMissionIndex = MAX(0, selectedMissionIndex - 1);
	
	if (selectedMissionIndex <= missionListStart + 3)
	{
		missionListStart = MAX(0, missionListStart - 1);
	}
	
	selectedMission = NULL;
}

static void nextMission(void)
{
	selectedMissionIndex = MIN(selectedMissionIndex + 1, selectedStarSystem->totalMissions - 1);
	
	if (selectedMissionIndex >= missionListStart + 5)
	{
		missionListStart = MIN(missionListStart + 1, selectedStarSystem->totalMissions - 9);
	}
	
	selectedMission = NULL;
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
		
	drawWidgets("galacticMap");
}

static void resume(void)
{
	show = SHOW_GALAXY;
}

static void options(void)
{
	show = SHOW_OPTIONS;
	
	initOptions(returnFromOptions);
}

static void stats(void)
{
	selectWidget("ok", "stats");
	
	show = SHOW_STATS;
}

static void statsOK(void)
{
	selectWidget("resume", "galacticMap");
	
	show = SHOW_MENU;
}

static void returnFromOptions(void)
{
	show = SHOW_MENU;
	
	selectWidget("resume", "galacticMap");
}

static void quit(void)
{
	initTitle();
}

void destroyGalacticMap(void)
{
	Pulse *pulse;
	
	while (pulseHead.next)
	{
		pulse = pulseHead.next;
		pulseHead.next = pulse->next;
		free(pulse);
	}
	
	pulseTail = &pulseHead;
}
