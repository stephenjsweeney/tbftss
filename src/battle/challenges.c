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

#include "challenges.h"

static void updateTimeChallenge(Challenge *c);
static void updateAccuracyChallenge(Challenge *c);
static void updateArmourChallenge(Challenge *c);
static void updateLossesChallenge(Challenge *c);
static void updatePlayerKillsChallenge(Challenge *c);
static char *getFormattedChallengeDescription(const char *format, ...);
char *getChallengeDescription(Challenge *c);

static char descriptionBuffer[MAX_DESCRIPTION_LENGTH];

static char *challengeDescription[] = {
	"Retain at least %d%% armour",
	"Finish mission in %d seconds or less",
	"Attain a %d%% hit accuracy",
	"Do not lose any team mates",
	"Do not lose more than 1 team mate",
	"Do not lose more than %d team mates",
	"Take down %d enemy targets"
};

void updateChallenges(void)
{
	Challenge *c;
	
	for (c = game.currentMission->challengeHead.next ; c != NULL ; c = c->next)
	{
		if (!c->passed)
		{
			switch (c->type)
			{
				case CHALLENGE_TIME:
					updateTimeChallenge(c);
					break;
					
				case CHALLENGE_ACCURACY:
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
			}
		}
	}
}

static void updateTimeChallenge(Challenge *c)
{
	if (battle.stats.time / FPS <= c->targetValue)
	{
		c->passed = 1;
	}
}

static void updateAccuracyChallenge(Challenge *c)
{
	float percent;
	
	percent = battle.stats.shotsHit;
	percent /= battle.stats.shotsFired;
	percent *= 100;
	
	if (percent >= c->targetValue)
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
	
	if (percent >= c->targetValue)
	{
		c->passed = 1;
	}
}

static void updateLossesChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats.alliesKilled <= c->targetValue;
	}
}

static void updatePlayerKillsChallenge(Challenge *c)
{
	if (!c->passed)
	{
		c->passed = battle.stats.playerKills >= c->targetValue;
	}
}

char *getChallengeDescription(Challenge *c)
{
	return getFormattedChallengeDescription(challengeDescription[c->type], c->targetValue);
}

Challenge *getChallenge(Mission *mission, int type)
{
	Challenge *challenge;
	
	for (challenge = mission->challengeHead.next ; challenge != NULL ; challenge = challenge->next)
	{
		if (challenge->type == type)
		{
			return challenge;
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
