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

#include "trophies.h"

static void prevPage(void);
static void nextPage(void);
static void loadTrophyData(char *filename);
static void resetAlert(void);
static void setSparkleColor(Trophy *t);
static void nextAlert(void);

static Trophy *alertTrophy;
static AtlasImage *trophyIcons[TROPHY_MAX];
static AtlasImage *sparkle;
static AtlasImage *alertSphere;
static SDL_Rect alertRect;
static int alertTimer;
static int page;
static int awarded;
static int total;
static int boxWidth;
static float sparkleAngle;
static float maxPages;
static Widget *prev;
static Widget *next;
static char *TROPHIES_TEXT;
static char *AWARDED_TEXT;
static char *PAGE_TEXT;
static char *HIDDEN_TEXT;

void initTrophies(void)
{
	loadTrophyData("data/trophies/trophies.json");

	trophyIcons[TROPHY_BRONZE] = getAtlasImage("gfx/trophies/bronze.png");
	trophyIcons[TROPHY_SILVER] = getAtlasImage("gfx/trophies/silver.png");
	trophyIcons[TROPHY_GOLD] = getAtlasImage("gfx/trophies/gold.png");
	trophyIcons[TROPHY_PLATINUM] = getAtlasImage("gfx/trophies/platinum.png");
	trophyIcons[TROPHY_UNEARNED] = getAtlasImage("gfx/trophies/unearned.png");
	sparkle = getAtlasImage("gfx/trophies/sparkle.png");
	alertSphere = getAtlasImage("gfx/trophies/alertSphere.png");
	
	alertRect.h = 90;
	alertRect.y = 10;
	
	sparkleAngle = 0;
	
	TROPHIES_TEXT = _("Trophies");
	AWARDED_TEXT = _("Awarded : %d / %d");
	PAGE_TEXT = _("Page : %d / %d");
	HIDDEN_TEXT = _("[Hidden]");

	resetAlert();
}

void initTrophiesDisplay(void)
{
	int w, h;
	Trophy *t;
	
	boxWidth = total = awarded = 0;
	
	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		total++;
		
		if (t->awarded)
		{
			awarded++;
			
			STRNCPY(t->awardDateStr, timeToDate(t->awardDate), MAX_NAME_LENGTH);
		}
		
		calcTextDimensions(t->description, 18, &w, &h);
		
		boxWidth = MAX(boxWidth, w);
	}
	
	boxWidth += 125;
	
	page = 0;
	
	maxPages = total;
	maxPages /= TROPHIES_PER_PAGE;
	maxPages = ceil(maxPages);
	
	prev = getWidget("prev", "trophies");
	prev->action = prevPage;
	prev->visible = 0;
	
	next = getWidget("next", "trophies");
	next->action = nextPage;
	next->visible = 1;
}

static void nextPage(void)
{
	page = MIN(page + 1, maxPages - 1);
	
	next->visible = page < maxPages - 1;
	prev->visible = 1;
}

static void prevPage(void)
{
	page = MAX(0, page - 1);
	
	next->visible = 1;
	prev->visible = page > 0;
}

void drawTrophies(void)
{
	Trophy *t;
	SDL_Rect r;
	int start, end, i, x, y;
	
	SDL_SetRenderTarget(app.renderer, app.uiBuffer);
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = boxWidth;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 40, 28, TA_CENTER, colors.white, TROPHIES_TEXT);
	drawText(SCREEN_WIDTH / 2, 83, 16, TA_CENTER, colors.lightGrey, AWARDED_TEXT, awarded, total);
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, PAGE_TEXT, page + 1, (int)maxPages);
	
	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, 150, r.x + r.w, 150);
	
	x = r.x + 15;
	y = 180;
	start = page * TROPHIES_PER_PAGE;
	end = start + TROPHIES_PER_PAGE;
	i = 0;
	
	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (i >= start && i < end)
		{
			if (t->awarded)
			{
				setSparkleColor(t);
				blitRotated(sparkle, x + 32, y + 32, sparkleAngle);
				blitRotated(sparkle, x + 32, y + 32, -sparkleAngle);
				
				setAtlasColor(255, 255, 255, 255);
				blitScaled(trophyIcons[t->value], x, y, 64, 64, 0);
				drawText(x + 85, y - 10, 20, TA_LEFT, colors.yellow, t->title);
				drawText(x + 85, y + 20, 18, TA_LEFT, colors.white, t->description);
				drawText(x + 85, y + 48, 18, TA_LEFT, colors.white, t->awardDateStr);
			}
			else
			{
				if (!t->hidden)
				{
					setAtlasColor(255, 255, 255, 128);
					blitScaled(trophyIcons[t->value], x, y, 64, 64, 0);
					drawText(x + 85, y - 10, 20, TA_LEFT, colors.lightGrey, t->title);
					drawText(x + 85, y + 20, 18, TA_LEFT, colors.darkGrey, t->description);
					drawText(x + 85, y + 48, 18, TA_LEFT, colors.darkGrey, "-");
				}
				else
				{
					blitScaled(trophyIcons[TROPHY_UNEARNED], x, y, 64, 64, 0);
					drawText(x + 85, y + 20, 20, TA_LEFT, colors.darkGrey, HIDDEN_TEXT);
				}
			}
			
			y += 120;
		}
		
		i++;
	}
		
	drawWidgets("trophies");
	
	SDL_SetRenderTarget(app.renderer, app.backBuffer);
}

void awardTrophy(char *id)
{
	Trophy *t;
	int numRemaining;
	
	numRemaining = 0;
	
	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (!t->awarded && strcmp(t->id, id) == 0)
		{
			t->awarded = 1;
			t->awardDate = time(NULL);
			t->notify = SDL_GetTicks();
			
			/* prevent race condition */
			SDL_Delay(1);
			
			SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Awarding trophy '%s'", t->id);
			
			app.saveGame = 1;
		}
		
		if (!t->awarded)
		{
			numRemaining++;
		}
	}
	
	/* the Platinum will always be the last trophy to unlock */
	if (numRemaining == 1)
	{
		awardTrophy("PLATINUM");
	}
}

void doTrophyAlerts(void)
{
	if (!alertTrophy)
	{
		nextAlert();
	}
	else if (alertTrophy)
	{
		alertRect.x = MIN(alertRect.x + 24, -1);

		if (alertRect.x > -150)
		{
			alertTimer--;
		}

		if (alertTimer <= 0)
		{
			alertTrophy->notify = 0;
			resetAlert();
		}
	}
	
	sparkleAngle = mod(sparkleAngle + 0.25, 360);
}

static void nextAlert(void)
{
	int w, h;
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (t->notify)
		{
			if (!alertTrophy || t->notify < alertTrophy->notify)
			{
				alertTrophy = t;
			}
		}
	}
	
	if (alertTrophy)
	{
		playSound(SND_TROPHY);
		
		calcTextDimensions(alertTrophy->title, 30, &alertRect.w, &h);
		
		calcTextDimensions(alertTrophy->description, 20, &w, &h);
		
		alertRect.w = MAX(alertRect.w, w);
		alertRect.w = MAX(400, alertRect.w);
		alertRect.w += 125;
		alertRect.x = -alertRect.w;
	}
}

static void resetAlert(void)
{
	alertTimer = FPS * 3;
	alertTrophy = NULL;
}

void drawTrophyAlert(void)
{
	int x, y;
	
	if (alertTrophy)
	{
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(app.renderer, &alertRect);

		SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, 255);
		SDL_RenderDrawRect(app.renderer, &alertRect);

		drawText(alertRect.x + 15, alertRect.y + 5, 30, TA_LEFT, colors.white, alertTrophy->title);
		drawText(alertRect.x + 15, alertRect.y + 45, 20, TA_LEFT, colors.white, alertTrophy->description);
		
		setSparkleColor(alertTrophy);
		
		x = alertRect.x + alertRect.w - 72;
		y = alertRect.y + 20;

		blit(alertSphere, x + 24, y + 24, 1);
		blitRotated(sparkle, x + 24, y + 24, sparkleAngle);
		blitRotated(sparkle, x + 24, y + 24, -sparkleAngle);
		blitScaled(trophyIcons[alertTrophy->value], x, y, 48, 48, 0);
	}
}

Trophy *getTrophy(char *id)
{
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (strcmp(t->id, id) == 0)
		{
			return t;
		}
	}

	return NULL;
}

static void loadTrophyData(char *filename)
{
	cJSON *root, *node;
	char *text;
	Trophy *t, *tail;
	int count[TROPHY_MAX];

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);
	root = cJSON_Parse(text);

	tail = &game.trophyHead;
	
	memset(count, 0, sizeof(int) * TROPHY_MAX);

	for (node = root->child ; node != NULL ; node = node->next)
	{
		if (cJSON_GetObjectItem(node, "id")->valuestring[0] != '_')
		{
			t = malloc(sizeof(Trophy));
			memset(t, 0, sizeof(Trophy));

			STRNCPY(t->id, cJSON_GetObjectItem(node, "id")->valuestring, MAX_NAME_LENGTH);
			STRNCPY(t->title, _(cJSON_GetObjectItem(node, "title")->valuestring), MAX_DESCRIPTION_LENGTH);
			STRNCPY(t->description, _(cJSON_GetObjectItem(node, "description")->valuestring), MAX_DESCRIPTION_LENGTH);
			t->value = lookup(cJSON_GetObjectItem(node, "value")->valuestring);
			t->hidden = getJSONValue(node, "hidden", 0);
			
			t->stat = -1;
			
			/* can't use the getJSONValue here, as it could lead to false positives */
			if (cJSON_GetObjectItem(node, "stat"))
			{
				t->stat = lookup(cJSON_GetObjectItem(node, "stat")->valuestring);
				t->statValue = cJSON_GetObjectItem(node, "statValue")->valueint;
			}
			
			count[t->value]++;
			count[TROPHY_UNEARNED]++;

			tail->next = t;
			tail = t;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Trophies (%d) [Bronze=%d, Silver=%d, Gold=%d, Platinum=%d]", count[TROPHY_UNEARNED], count[TROPHY_BRONZE], count[TROPHY_SILVER], count[TROPHY_GOLD], count[TROPHY_PLATINUM]);

	cJSON_Delete(root);
	free(text);
}

void awardStatsTrophies(void)
{
	Trophy *t;
	Tuple *tp;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (t->stat != -1 && !t->awarded && (game.stats[t->stat] + battle.stats[t->stat]) >= t->statValue)
		{
			awardTrophy(t->id);
		}
	}
	
	/* check to see if we've destroyed one of each common starfighter */
	for (tp = game.fighterStatHead.next ; tp != NULL ; tp = tp->next)
	{
		if (tp->value == 0)
		{
			return;
		}
	}
	
	awardTrophy("FREQUENT_FLYER");
}

void awardCampaignTrophies(void)
{
	char trophyId[MAX_NAME_LENGTH];
	char name[MAX_NAME_LENGTH];
	int i, len;
	StarSystem *starSystem;
	
	if (game.completedMissions)
	{
		awardTrophy("CAMPAIGN_1");
	}

	/* check if all star system missions are completed */
	for (starSystem = game.starSystemHead.next ; starSystem != NULL ; starSystem = starSystem->next)
	{
		if (starSystem->totalMissions && starSystem->completedMissions == starSystem->totalMissions)
		{
			memset(name, '\0', MAX_NAME_LENGTH);

			len = strlen(starSystem->name);

			for (i = 0 ; i < len ; i++)
			{
				name[i] = toupper(starSystem->name[i]);
			}
			
			sprintf(trophyId, "CAMPAIGN_%s", name);
			awardTrophy(trophyId);
		}
	}
}

void awardChallengeTrophies(void)
{
	char trophyId[MAX_NAME_LENGTH];
	int completed;

	/* check % of challenges completed - 25% increments*/
	completed = (getPercent(game.completedChallenges, game.totalChallenges) / 25) * 25;
	sprintf(trophyId, "CHALLENGE_%d", completed);
	awardTrophy(trophyId);
}

void awardPostMissionTrophies(void)
{
	if (game.currentMission->epic)
	{
		awardTrophy("EPIC");
		
		if (battle.stats[STAT_PLAYER_KILLED] == 0 && player->flags & EF_COMMON_FIGHTER)
		{
			awardTrophy("SURVIVOR");
		}
	}
	
	/*
	 * Must be a non-challenge mission, a common fighter, must not be Sol, and must not have fired any shots or missiles (and there should have been some enemies present)
	 */
	if (player->flags & EF_COMMON_FIGHTER && player->missiles && strcmp(game.selectedStarSystem, "Sol") && !battle.stats[STAT_SHOTS_FIRED] && !battle.stats[STAT_MISSILES_FIRED] && battle.numInitialEnemies > 0)
	{
		awardTrophy("PACIFIST");
	}
}

void awardCraftTrophy(void)
{
	if (!game.currentMission->challengeData.isChallenge)
	{
		if (strcmp(game.currentMission->craft, "ATAF") == 0)
		{
			awardTrophy("ATAF");
		}
		else if (strcmp(game.currentMission->craft, "Tug") == 0)
		{
			awardTrophy("TUG");
		}
	}
	else
	{
		if (strcmp(game.currentMission->craft, "Shuttle") == 0 && battle.stats[STAT_ITEMS_COLLECTED_PLAYER] > 0)
		{
			awardTrophy("SHUTTLE");
		}
	}
	
	awardPandoranCraftTrophy();
}

static void setSparkleColor(Trophy *t)
{
	switch (t->value)
	{
		case TROPHY_BRONZE:
			setAtlasColor(255, 128, 0, 255);
			break;
		
		case TROPHY_SILVER:
			setAtlasColor(192, 192, 192, 255);
			break;
		
		case TROPHY_GOLD:
			setAtlasColor(255, 255, 0, 255);
			break;
		
		case TROPHY_PLATINUM:
			setAtlasColor(0, 128, 255, 255);
			break;
	}
}
