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

#include "../common.h"

#include "../json/cJSON.h"

#include "locale.h"

#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

extern void createSaveFolder(void);
extern void destroyBattle(void);
extern void destroyBulletDefs(void);
extern void destroyCapitalShipDefs(void);
extern void destroyCredits(void);
extern void destroyFighterDatabase(void);
extern void destroyFighterDefs(void);
extern void destroyFighterStats(void);
extern void destroyGalacticMap(void);
extern void destroyGame(void);
extern void destroyItemDefs(void);
extern void destroyLookups(void);
extern void destroyResources(void);
extern void destroySounds(void);
extern void destroyStarSystems(void);
extern void destroyTextures(void);
extern void destroyWidgets(void);
extern int fileExists(char *filename);
extern int getJSONValue(cJSON *node, char *name, int defValue);
extern char *getLookupName(char *prefix, long num);
extern char *getSaveFilePath(char *filename);
extern void initAtlas(void);
extern void initBackground(void);
extern void initBulletDefs(void);
extern void initChallenges(void);
extern void initControls(void);
extern void initFighterDatabase(void);
extern void initFonts(void);
extern void initGame(void);
extern void initInput(void);
extern void initModalDialog(void);
extern void initResources(void);
extern void initSounds(void);
extern void initStarSystems(void);
extern void initStars(void);
extern void initStats(void);
extern void initTrophies(void);
extern void initWidgets(void);
extern void loadCapitalShipDefs(void);
extern void loadFighterDefs(void);
extern void loadItemDefs(void);
extern long lookup(char *name);
extern void prepareScene(void);
extern void presentScene(void);
extern char *readFile(char *filename);
extern void setLanguage(char *applicationName, char *languageCode);
extern int writeFile(char *filename, char *data);

extern App app;
extern Colors colors;
