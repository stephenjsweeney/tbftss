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

#include "SDL2/SDL.h"
#include "time.h"

#include "defs.h"
#include "structs.h"

extern void cleanup(void);
extern void clearControl(int type);
extern void createScreenshotFolder(void);
extern void doDevKeys(void);
extern void doInput(void);
extern void doModalDialog(void);
extern void doTrophyAlerts(void);
extern void drawModalDialog(void);
extern void drawTrophyAlert(void);
extern int fileExists(char *filename);
extern char *getSaveFilePath(char *filename);
extern void init18N(int argc, char *argv[]);
extern void initCredits(void);
extern void initGameSystem(void);
extern void initLookups(void);
extern void initSDL(int argc, char *argv[]);
extern void initTitle(void);
extern int isControl(int type);
extern void loadGame(void);
extern void loadTestMission(char *filename);
extern void prepareScene(void);
extern void presentScene(void);
extern void saveGame(void);
extern void saveScreenshot(void);

App app;
Battle battle;
Colors colors;
Dev dev;
Entity *player;
Entity *self;
Game game;
