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

#include "trophies.h"

static void prevPage(void);
static void nextPage(void);
static void loadTrophyData(char *filename);
static void resetAlert(void);
static void awardCraftTrophy(void);
static void setSparkleColor(Trophy *t);
static void nextAlert(void);

static Trophy *alertTrophy;
static SDL_Texture *trophyIcons[TROPHY_MAX];
static SDL_Texture *sparkle;
static SDL_Texture *alertSphere;
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

void initTrophies(void)
{
	loadTrophyData("data/trophies/trophies.json");

	trophyIcons[TROPHY_BRONZE] = getTexture("gfx/trophies/bronze.png");
	trophyIcons[TROPHY_SILVER] = getTexture("gfx/trophies/silver.png");
	trophyIcons[TROPHY_GOLD] = getTexture("gfx/trophies/gold.png");
	trophyIcons[TROPHY_PLATINUM] = getTexture("gfx/trophies/platinum.png");
	trophyIcons[TROPHY_UNEARNED] = getTexture("gfx/trophies/unearned.png");
	sparkle = getTexture("gfx/trophies/sparkle.png");
	alertSphere = getTexture("gfx/trophies/alertSphere.png");
	
	alertRect.h = 90;
	alertRect.y = 10;

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
		
		textSize(t->description, 18, &w, &h);
		
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
	
	sparkleAngle = 0;
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

void doTrophies(void)
{
	sparkleAngle += 0.25;
	if (sparkleAngle >= 360)
	{
		sparkleAngle = 0;
	}
}

void drawTrophies(void)
{
	Trophy *t;
	SDL_Rect r;
	int start, i, x, y;
	
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 128);
	SDL_RenderFillRect(app.renderer, NULL);
	SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
	
	r.w = boxWidth;
	r.h = 650;
	r.x = (SCREEN_WIDTH / 2) - r.w / 2;
	r.y = (SCREEN_HEIGHT / 2) - r.h / 2;
	
	SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(app.renderer, &r);
	SDL_SetRenderDrawColor(app.renderer, 200, 200, 200, 255);
	SDL_RenderDrawRect(app.renderer, &r);
	
	drawText(SCREEN_WIDTH / 2, 40, 28, TA_CENTER, colors.white, _("Trophies"));
	drawText(SCREEN_WIDTH / 2, 83, 16, TA_CENTER, colors.lightGrey, _("Awarded : %d / %d"), awarded, total);
	drawText(SCREEN_WIDTH / 2, 110, 16, TA_CENTER, colors.lightGrey, _("Page : %d / %d"), page + 1, (int)maxPages);
	
	SDL_SetRenderDrawColor(app.renderer, 128, 128, 128, 255);
	SDL_RenderDrawLine(app.renderer, r.x, 150, r.x + r.w, 150);
	
	x = r.x + 15;
	y = 180;
	start = page * TROPHIES_PER_PAGE;
	i = 0;
	
	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (i >= start && i < start + TROPHIES_PER_PAGE)
		{
			if (t->awarded)
			{
				setSparkleColor(t);
				blitRotated(sparkle, x + 32, y + 32, sparkleAngle);
				blitRotated(sparkle, x + 32, y + 32, -sparkleAngle);
				
				blitScaled(trophyIcons[t->value], x, y, 64, 64);
				drawText(x + 85, y - 10, 20, TA_LEFT, colors.white, t->title);
				drawText(x + 85, y + 20, 18, TA_LEFT, colors.lightGrey, t->description);
				drawText(x + 85, y + 48, 18, TA_LEFT, colors.lightGrey, t->awardDateStr);
			}
			else
			{
				blitScaled(trophyIcons[TROPHY_UNEARNED], x, y, 64, 64);
				
				if (!t->hidden)
				{
					drawText(x + 85, y - 10, 20, TA_LEFT, colors.white, t->title);
					drawText(x + 85, y + 20, 18, TA_LEFT, colors.lightGrey, t->description);
					drawText(x + 85, y + 48, 18, TA_LEFT, colors.lightGrey, "-");
				}
				else
				{
					drawText(x + 85, y + 20, 20, TA_LEFT, colors.darkGrey, _("[Hidden]"));
				}
			}
			
			y += 120;
		}
		
		i++;
	}
		
	drawWidgets("trophies");
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
			t->notify = 1;
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
		alertRect.x = MIN(alertRect.x + 16, -1);

		if (alertRect.x > -150)
		{
			alertTimer--;
		}

		if (alertTimer <= 0)
		{
			alertTrophy->notify = 0;
			resetAlert();
		}
		
		/* do the sparkle rotation */
		doTrophies();
	}
}

static void nextAlert(void)
{
	int h;
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (t->notify)
		{
			if (!alertTrophy || t->awardDate < alertTrophy->awardDate)
			{
				alertTrophy = t;
			}
		}
	}
	
	if (alertTrophy)
	{
		playSound(SND_TROPHY);
		
		textSize(alertTrophy->title, 30, &alertRect.w, &h);
		alertRect.w += 125;
		alertRect.w = MAX(alertRect.w, 500);
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
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(app.renderer, &alertRect);

		SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(app.renderer, &alertRect);

		drawText(alertRect.x + 15, alertRect.y + 5, 30, TA_LEFT, colors.white, alertTrophy->title);
		drawText(alertRect.x + 15, alertRect.y + 45, 20, TA_LEFT, colors.white, alertTrophy->shortDescription);
		
		setSparkleColor(alertTrophy);
		
		x = alertRect.x + alertRect.w - 72;
		y = alertRect.y + 20;

		blit(alertSphere, x + 24, y + 24, 1);
		blitRotated(sparkle, x + 24, y + 24, sparkleAngle);
		blitRotated(sparkle, x + 24, y + 24, -sparkleAngle);
		blitScaled(trophyIcons[alertTrophy->value], x, y, 48, 48);
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

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);
	root = cJSON_Parse(text);

	tail = &game.trophyHead;

	for (node = root->child ; node != NULL ; node = node->next)
	{
		if (cJSON_GetObjectItem(node, "id")->valuestring[0] != '_')
		{
			t = malloc(sizeof(Trophy));
			memset(t, 0, sizeof(Trophy));

			STRNCPY(t->id, cJSON_GetObjectItem(node, "id")->valuestring, MAX_NAME_LENGTH);
			STRNCPY(t->title, _(cJSON_GetObjectItem(node, "title")->valuestring), MAX_DESCRIPTION_LENGTH);
			STRNCPY(t->description, _(cJSON_GetObjectItem(node, "description")->valuestring), MAX_DESCRIPTION_LENGTH);
			STRNCPY(t->shortDescription, _(cJSON_GetObjectItem(node, "description")->valuestring), MAX_NAME_LENGTH);
			t->value = lookup(cJSON_GetObjectItem(node, "value")->valuestring);
			t->hidden = getJSONValue(node, "hidden", 0);
			
			if (strlen(t->description) > MAX_NAME_LENGTH)
			{
				t->shortDescription[MAX_NAME_LENGTH - 1] = '.';
				t->shortDescription[MAX_NAME_LENGTH - 2] = '.';
				t->shortDescription[MAX_NAME_LENGTH - 3] = '.';
			}
			
			t->stat = -1;
			
			/* can't use the getJSONValue here, as it could lead to false positives */
			if (cJSON_GetObjectItem(node, "stat"))
			{
				t->stat = lookup(cJSON_GetObjectItem(node, "stat")->valuestring);
				t->statValue = cJSON_GetObjectItem(node, "statValue")->valueint;
			}

			tail->next = t;
			tail = t;
		}
	}

	cJSON_Delete(root);
	free(text);
}

void awardStatsTrophies(void)
{
	Trophy *t;

	for (t = game.trophyHead.next ; t != NULL ; t = t->next)
	{
		if (t->stat != -1 && !t->awarded && (game.stats[t->stat] + battle.stats[t->stat]) >= t->statValue)
		{
			t->awarded = 1;
			t->awardDate = time(NULL);
			t->notify = 1;
		}
	}
}

void awardCampaignTrophies(void)
{
	char trophyId[MAX_NAME_LENGTH];
	char name[MAX_NAME_LENGTH];
	int completed, i, len;
	StarSystem *starSystem;
	
	if (game.completedMissions)
	{
		awardTrophy("CAMPAIGN_1");
	}

	/* check % of missions completed */
	completed = getPercent(game.completedMissions, game.totalMissions);
	sprintf(trophyId, "CAMPAIGN_%d", completed);
	awardTrophy(trophyId);

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

	/* check % of challenges completed */
	completed = getPercent(game.completedChallenges, game.totalChallenges);
	sprintf(trophyId, "CHALLENGE_%d", completed);
	awardTrophy(trophyId);
}

void awardPostMissionTrophies(void)
{
	awardCraftTrophy();

	if (game.currentMission->epic)
	{
		awardTrophy("EPIC");
		
		if (battle.stats[STAT_PLAYER_KILLED] == 0)
		{
			awardTrophy("SURVIVOR");
		}
	}
}

/* the player is known as "Player", so we need to check the craft they were assigned to */
static void awardCraftTrophy(void)
{
	char trophyId[MAX_NAME_LENGTH];
	int len, i;

	memset(trophyId, '\0', MAX_NAME_LENGTH);

	len = strlen(game.currentMission->craft);

	for (i = 0 ; i < len ; i++)
	{
		trophyId[i] = toupper(game.currentMission->craft[i]);
	}

	awardTrophy(trophyId);
}

static void setSparkleColor(Trophy *t)
{
	switch (t->value)
	{
		case TROPHY_BRONZE:
			SDL_SetTextureColorMod(sparkle, 255, 128, 0);
			break;
		
		case TROPHY_SILVER:
			SDL_SetTextureColorMod(sparkle, 192, 192, 192);
			break;
		
		case TROPHY_GOLD:
			SDL_SetTextureColorMod(sparkle, 255, 255, 0);
			break;
		
		case TROPHY_PLATINUM:
			SDL_SetTextureColorMod(sparkle, 0, 128, 255);
			break;
	}
}
