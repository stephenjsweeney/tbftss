/*
Copyright (C) 2015-2017 Parallel Realities

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

#include "SDL2/SDL.h"
#include "time.h"

#include "defs.h"
#include "structs.h"

extern void cleanup(void);
extern void initSDL(void);
extern void initGameSystem(void);
extern void initTitle(void);
extern void loadTestMission(char *filename);
extern void saveScreenshot(void);
extern void doMouseDown(SDL_MouseButtonEvent *event);
extern void doMouseUp(SDL_MouseButtonEvent *event);
extern void doMouseWheel(SDL_MouseWheelEvent *event);
extern void doMouseMotion(SDL_MouseMotionEvent *event);
extern void doDevKeys(void);
extern void expireTexts(int all);
extern void prepareScene(void);
extern void presentScene(void);
extern void doModalDialog(void);
extern void drawModalDialog(void);
extern void loadGame(void);
extern int fileExists(char *filename);
extern char *getSaveFilePath(char *filename);
extern void init18N(int argc, char *argv[]);
extern void initLookups(void);
extern void doKeyDown(SDL_KeyboardEvent *event);
extern void doKeyUp(SDL_KeyboardEvent *event);
extern void createScreenshotFolder(void);
extern int isControl(int type);
extern void clearControl(int type);
extern void saveGame(void);
extern void initCredits(void);
extern void doTrophyAlerts(void);
extern void drawTrophyAlert(void);
extern void musicSetPlaying(int playing);

App app;
Colors colors;
Battle battle;
Dev dev;
Entity *self;
Entity *player;
Game game;
