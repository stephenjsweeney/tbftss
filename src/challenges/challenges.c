/*
Copyright (C) 2015-2019,2022 Parallel Realities

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

#include "../battle/fighters.h"
#include "../galaxy/mission.h"
#include "../game/stats.h"
#include "../game/trophies.h"
#include "../json/cJSON.h"
#include "../system/io.h"
#include "../system/lookup.h"
#include "../system/util.h"
#include "../system/widgets.h"
#include "challenges.h"

extern Battle  battle;
extern Dev	   dev;
extern Entity *player;
extern Game	   game;

static void	 updateTimeChallenge(Challenge *c);
static void	 updateSurvivalChallenge(Challenge *c);
static void	 updateAccuracyChallenge(Challenge *c);
static void	 updateArmourChallenge(Challenge *c);
static void	 updateLossesChallenge(Challenge *c);
static void	 updatePlayerKillsChallenge(Challenge *c);
static void	 updateDisabledChallenge(Challenge *c);
static void	 updateItemsChallenge(Challenge *c);
static void	 updateSurrenderChallenge(Challenge *c);
static void	 updateWaypointChallenge(Challenge *c);
static void	 updateRescueChallenge(Challenge *c);
static void	 completeChallenge(void);
static void	 failChallenge(void);
static int	 updateChallenges(void);
static char *getFormattedChallengeDescription(const char *format, ...);
static int	 challengeFinished(void);
static int	 alreadyPassed(void);
static void	 printStats(void);

static char		   descriptionBuffer[MAX_DESCRIPTION_LENGTH];
static const char *challengeDescription[CHALLENGE_MAX];

void initChallenges(void)
{
	Mission *mission, *tail;
	char	 **filenames;
	char	 path[MAX_FILENAME_LENGTH];
	int		 count, i;

	challengeDescription[CHALLENGE_ARMOUR] = _("Retain at least %d%% armour");
	challengeDescription[CHALLENGE_TIME] = _("Complete challenge in %d seconds or less");
	challengeDescription[CHALLENGE_SURVIVE] = _("Survive for %d seconds");
	challengeDescription[CHALLENGE_SHOT_ACCURACY] = _("Attain a %d%% shot hit accuracy");
	challengeDescription[CHALLENGE_ROCKET_ACCURACY] = _("Attain a %d%% rocket hit accuracy");
	challengeDescription[CHALLENGE_MISSILE_ACCURACY] = _("Attain a %d%% missile hit accuracy");
	challengeDescription[CHALLENGE_NO_LOSSES] = _("Do not lose any team mates");
	challengeDescription[CHALLENGE_1_LOSS] = _("Do not lose more than 1 team mate");
	challengeDescription[CHALLENGE_LOSSES] = _("Do not lose more than %d team mates");
	challengeDescription[CHALLENGE_PLAYER_KILLS] = _("Take down %d enemy targets");
	challengeDescription[CHALLENGE_DISABLE] = _("Disable %d or more enemy fighters");
	challengeDescription[CHALLENGE_ITEMS] = _("Collect %d packages");
	challengeDescription[CHALLENGE_PLAYER_ITEMS] = _("Collect %d packages");
	challengeDescription[CHALLENGE_RESCUE] = _("Rescue %d civilians");
	challengeDescription[CHALLENGE_SURRENDER] = _("Cause %d enemies to surrender");
	challengeDescription[CHALLENGE_WAYPOINTS] = _("Reach %d waypoints");

	tail = &game.challengeMissionHead;

	filenames = getFileList("data/challenges", &count);

	for (i = 0; i < count; i++)
	{
		sprintf(path, "data/challenges/%s", filenames[i]);

		mission = loadMissionMeta(path);

		if (mission)
		{
			tail->next = mission;
			tail = mission;
		}

		free(filenames[i]);
	}

	free(filenames);
}

void loadChallenge(Mission *mission, cJSON *node)
{
	int		   i;
	Challenge *challenge;

	mission->challengeData.isChallenge = 1;

	/* limits */
	mission->challengeData.timeLimit = getJSONValue(node, "timeLimit", 0) * FPS;
	mission->challengeData.killLimit = getJSONValue(node, "killLimit", 0);
	mission->challengeData.escapeLimit = getJSONValue(node, "escapeLimit", 0);
	mission->challengeData.waypointLimit = getJSONValue(node, "waypointLimit", 0);
	mission->challengeData.itemLimit = getJSONValue(node, "itemLimit", 0);
	mission->challengeData.playerItemLimit = getJSONValue(node, "playerItemLimit", 0);
	mission->challengeData.rescueLimit = getJSONValue(node, "rescueLimit", 0);
	mission->challengeData.disableLimit = getJSONValue(node, "disableLimit", 0);
	mission->challengeData.surrenderLimit = getJSONValue(node, "surrenderLimit", 0);

	/* restrictions */
	mission->challengeData.noMissiles = getJSONValue(node, "noMissiles", 0);
	mission->challengeData.noECM = getJSONValue(node, "noECM", 0);
	mission->challengeData.noBoost = getJSONValue(node, "noBoost", 0);
	mission->challengeData.noGuns = getJSONValue(node, "noGuns", 0);

	if (getJSONValue(node, "noWeapons", 0))
	{
		mission->challengeData.noMissiles = mission->challengeData.noGuns = 1;
	}

	/* misc */
	mission->challengeData.allowPlayerDeath = getJSONValue(node, "allowPlayerDeath", 0);
	mission->challengeData.clearWaypointEnemies = getJSONValue(node, "clearWaypointEnemies", 0);
	mission->challengeData.eliminateThreats = getJSONValue(node, "eliminateThreats", 0);
	mission->challengeData.isDeathMatch = getJSONValue(node, "isDeathMatch", 0);

	node = cJSON_GetObjectItem(node, "challenges");

	if (node)
	{
		node = node->child;

		i = 0;

		while (node && i < MAX_CHALLENGES)
		{
			challenge = malloc(sizeof(Challenge));
			memset(challenge, 0, sizeof(Challenge));

			challenge->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			challenge->value = cJSON_GetObjectItem(node, "value")->valueint;

			mission->challengeData.challenges[i] = challenge;

			node = node->next;

			i++;
		}
	}
}

void doChallenges(void)
{
	int passed;

	if (game.currentMission->challengeData.isChallenge && battle.status == MS_IN_PROGRESS)
	{
		if (challengeFinished())
		{
			passed = 0;

			if (player->health > 0 || (player->health <= 0 && game.currentMission->challengeData.allowPlayerDeath))
			{
				passed = updateChallenges();
			}

			if (passed)
			{
				completeChallenge();
			}
			else
			{
				failChallenge();
			}
		}
	}
}

static int challengeFinished(void)
{
	if (game.currentMission->challengeData.timeLimit > 0 && battle.stats[STAT_TIME] >= game.currentMission->challengeData.timeLimit)
	{
		return 1;
	}

	/* disabled enemies count as killed during challenges - not player exclusive, but no need to worry about AI contributions here */
	if (game.currentMission->challengeData.killLimit > 0 && (battle.stats[STAT_ENEMIES_KILLED_PLAYER] + battle.stats[STAT_CAPITAL_SHIPS_DESTROYED] + battle.stats[STAT_ENEMIES_DISABLED]) >= game.currentMission->challengeData.killLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.escapeLimit > 0 && (battle.stats[STAT_ENEMIES_KILLED_PLAYER] + battle.stats[STAT_ENEMIES_ESCAPED]) >= game.currentMission->challengeData.escapeLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.waypointLimit > 0 && battle.stats[STAT_WAYPOINTS_VISITED] >= game.currentMission->challengeData.waypointLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.itemLimit > 0 && battle.stats[STAT_ITEMS_COLLECTED] + battle.stats[STAT_ITEMS_COLLECTED_PLAYER] >= game.currentMission->challengeData.itemLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.playerItemLimit > 0 && battle.stats[STAT_ITEMS_COLLECTED_PLAYER] >= game.currentMission->challengeData.playerItemLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.rescueLimit > 0 && (battle.stats[STAT_CIVILIANS_RESCUED] + battle.stats[STAT_CIVILIANS_KILLED]) >= game.currentMission->challengeData.rescueLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.surrenderLimit > 0 && battle.stats[STAT_ENEMIES_SURRENDERED] >= game.currentMission->challengeData.surrenderLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.waypointLimit > 0 && (battle.stats[STAT_WAYPOINTS_VISITED]) >= game.currentMission->challengeData.waypointLimit)
	{
		return 1;
	}

	if (game.currentMission->challengeData.eliminateThreats && !battle.hasThreats)
	{
		return 1;
	}

	return (player->health <= 0 || player->alive == ALIVE_ESCAPED || battle.scriptedEnd);
}

static int updateChallenges(void)
{
	int		   i, numPassed;
	Challenge *c;

	updateAccuracyStats(battle.stats);

	numPassed = 0;

	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		c = game.currentMission->challengeData.challenges[i];

		if (c)
		{
			if (!c->passed)
			{
				switch (c->type)
				{
					case CHALLENGE_TIME:
						updateTimeChallenge(c);
						break;

					case CHALLENGE_SURVIVE:
						updateSurvivalChallenge(c);
						break;

					case CHALLENGE_SHOT_ACCURACY:
					case CHALLENGE_ROCKET_ACCURACY:
					case CHALLENGE_MISSILE_ACCURACY:
						updateAccuracyChallenge(c);
						break;

					case CHALLENGE_ARMOUR:
						updateArmourChallenge(c);
						break;

					case CHALLENGE_NO_LOSSES:
					case CHALLENGE_1_LOSS:
					case CHALLENGE_LOSSES:
						updateLossesChallenge(c);
						break;

					case CHALLENGE_PLAYER_KILLS:
						updatePlayerKillsChallenge(c);
						break;

					case CHALLENGE_DISABLE:
						updateDisabledChallenge(c);
						break;

					case CHALLENGE_ITEMS:
					case CHALLENGE_PLAYER_ITEMS:
						updateItemsChallenge(c);
						break;

					case CHALLENGE_SURRENDER:
						updateSurrenderChallenge(c);
						break;

					case CHALLENGE_WAYPOINTS:
						updateWaypointChallenge(c);
						break;

					case CHALLENGE_RESCUE:
						updateRescueChallenge(c);
						break;
				}
			}

			if (c->passed)
			{
				numPassed++;
			}
		}
	}

	if (dev.debug)
	{
		printStats();
	}

	return numPassed;
}

static void printStats(void)
{
	int i;

	for (i = 0; i < STAT_MAX; i++)
	{
		if (battle.stats[i])
		{
			if (i != STAT_TIME)
			{
				printf("DEBUG: %s=%d\n", getLookupName("STAT_", i), battle.stats[i]);
			}
			else
			{
				printf("DEBUG: %s=%s\n", getLookupName("STAT_", i), timeToString(battle.stats[i], 0));
			}
		}
	}
}

static void updateTimeChallenge(Challenge *c)
{
	if (battle.stats[STAT_TIME] / FPS < c->value)
	{
		c->passed = 1;
	}
}

static void updateSurvivalChallenge(Challenge *c)
{
	if (battle.stats[STAT_TIME] / FPS >= c->value)
	{
		c->passed = 1;
	}
}

static void updateAccuracyChallenge(Challenge *c)
{
	float percent;

	switch (c->type)
	{
		case CHALLENGE_SHOT_ACCURACY:
			percent = battle.stats[STAT_SHOT_ACCURACY];
			break;

		case CHALLENGE_ROCKET_ACCURACY:
			percent = battle.stats[STAT_ROCKET_ACCURACY];
			break;

		case CHALLENGE_MISSILE_ACCURACY:
			percent = battle.stats[STAT_MISSILE_ACCURACY];
			break;

		default:
			percent = 0;
			break;
	}

	if (percent >= c->value)
	{
		c->passed = 1;
	}
}

static void updateArmourChallenge(Challenge *c)
{
	float percent;

	percent = player->health;
	percent /= player->maxHealth;
	percent *= 100;

	if (percent >= c->value)
	{
		c->passed = 1;
	}
}

static void updateLossesChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_ALLIES_KILLED] <= c->value;
	}
}

static void updatePlayerKillsChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_ENEMIES_KILLED_PLAYER] >= c->value;
	}
}

static void updateDisabledChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_ENEMIES_DISABLED] >= c->value;
	}
}

static void updateItemsChallenge(Challenge *c)
{
	if (!c->passed)
	{
		if (c->type == CHALLENGE_ITEMS)
		{
			c->passed = battle.stats[STAT_ITEMS_COLLECTED] + battle.stats[STAT_ITEMS_COLLECTED_PLAYER] >= c->value;
		}
		else
		{
			c->passed = battle.stats[STAT_ITEMS_COLLECTED_PLAYER] >= c->value;
		}
	}
}

static void updateSurrenderChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_ENEMIES_SURRENDERED] >= c->value;
	}
}

static void updateWaypointChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_WAYPOINTS_VISITED] >= c->value;
	}
}

static void updateRescueChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats[STAT_CIVILIANS_RESCUED] >= c->value;
	}
}

char *getChallengeDescription(Challenge *c)
{
	if (c->type == CHALLENGE_TIME)
	{
		if (c->value <= 90)
		{
			return getFormattedChallengeDescription(challengeDescription[CHALLENGE_TIME], c->value);
		}
		else
		{
			return getFormattedChallengeDescription(_("Complete challenge in %s or less"), timeToString(c->value * FPS, 0));
		}
	}

	return getFormattedChallengeDescription(challengeDescription[c->type], c->value);
}

Challenge *getChallenge(Mission *mission, int type, int value)
{
	int		   i;
	Challenge *c;

	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		c = mission->challengeData.challenges[i];

		if (c->type == type && c->value == value)
		{
			return c;
		}
	}

	return NULL;
}

static char *getFormattedChallengeDescription(const char *format, ...)
{
	va_list args;

	memset(&descriptionBuffer, '\0', sizeof(descriptionBuffer));

	va_start(args, format);
	vsprintf(descriptionBuffer, format, args);
	va_end(args);

	return descriptionBuffer;
}

void updateChallengeMissions(void)
{
	int		   i;
	Mission	*m;
	Challenge *c;

	for (m = game.challengeMissionHead.next; m != NULL; m = m->next)
	{
		m->totalChallenges = m->completedChallenges = 0;

		for (i = 0; i < MAX_CHALLENGES; i++)
		{
			c = m->challengeData.challenges[i];

			if (c)
			{
				m->totalChallenges++;

				if (c->passed)
				{
					m->completedChallenges++;
				}
			}
		}
	}
}

static void completeChallenge(void)
{
	if (battle.status == MS_IN_PROGRESS)
	{
		battle.status = MS_COMPLETE;
		battle.missionFinishedTimer = FPS;
		selectWidget("continue", "battleWon");

		game.stats[STAT_CHALLENGES_COMPLETED]++;

		player->flags |= EF_IMMORTAL;

		retreatAllies();

		retreatEnemies();

		awardStatsTrophies();

		awardCraftTrophy();
	}
}

static void failChallenge(void)
{
	if (battle.status == MS_IN_PROGRESS)
	{
		battle.status = MS_FAILED;
		battle.missionFinishedTimer = FPS;
		selectWidget("retry", "battleLost");

		player->flags |= EF_IMMORTAL;

		if (alreadyPassed())
		{
			battle.status = MS_TIME_UP;
		}

		retreatAllies();

		retreatEnemies();

		awardStatsTrophies();
	}
}

static int alreadyPassed(void)
{
	int		   i;
	Challenge *c;

	for (i = 0; i < MAX_CHALLENGES; i++)
	{
		c = game.currentMission->challengeData.challenges[i];

		if (c && c->passed)
		{
			return 1;
		}
	}

	return 0;
}
