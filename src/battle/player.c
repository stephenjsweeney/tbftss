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

#include "player.h"

static void selectTarget(void);
static void switchGuns(void);
static void cycleRadarZoom(void);
static void selectMissionTarget(void);
static void selectNewPlayer(int dir);
static void initPlayerSelect(void);
static void activateBoost(void);
static void deactivateBoost(void);
static void activateECM(void);
static void handleKeyboard(void);
static void faceMouse(void);
static void handleMouse(void);
static void preFireMissile(void);
static void applyRestrictions(void);
static int isPriorityMissionTarget(Entity *e, int dist, int closest);
static int targetOutOfRange(void);
static void rechargeBoostECM(void);
static void setPilotName(void);

static int selectedPlayerIndex;
static int availableGuns[BT_MAX];
static Entity *availablePlayerUnits[MAX_SELECTABLE_PLAYERS];

void initPlayer(void)
{
	int i, n;

	memset(&availableGuns, 0, sizeof(int) * BT_MAX);

	battle.numPlayerGuns = 0;

	player->selectedGunType = -1;

	if (!player->combinedGuns)
	{
		for (i = 0 ; i < MAX_FIGHTER_GUNS ; i++)
		{
			n = player->guns[i].type;

			if (n)
			{
				if (!availableGuns[n])
				{
					battle.numPlayerGuns++;
				}

				availableGuns[n] = 1;

				if (player->selectedGunType == -1)
				{
					player->selectedGunType = n;
				}
			}
		}
	}
	else
	{
		player->selectedGunType = 0;
	}
	
	setPilotName();

	player->action = NULL;

	battle.boostTimer = BOOST_RECHARGE_TIME;
	battle.ecmTimer = ECM_RECHARGE_TIME;
	
	player->flags |= EF_NO_HEALTH_BAR;
	player->flags &= ~EF_IMMORTAL;
	
	game.stats[STAT_EPIC_KILL_STREAK] = MAX(game.stats[STAT_EPIC_KILL_STREAK], battle.stats[STAT_EPIC_KILL_STREAK]);
	
	battle.stats[STAT_EPIC_KILL_STREAK] = 0;
}

static void setPilotName(void)
{
	int i, pos;
	
	pos = -1;
	
	for (i = 0 ; i < strlen(game.currentMission->pilot) ; i++)
	{
		if (game.currentMission->pilot[i] == ' ')
		{
			pos = i;
		}
	}
	
	memset(player->name, '\0', MAX_NAME_LENGTH);
	
	if (pos != -1)
	{
		memcpy(player->name, game.currentMission->pilot + pos + 1, strlen(game.currentMission->pilot) - pos - 1);
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Pilot name = '%s'", player->name);
}

void doPlayer(void)
{
	self = player;
	
	rechargeBoostECM();
	
	if (game.currentMission->challengeData.isChallenge)
	{
		applyRestrictions();
	}
	
	if (player->alive == ALIVE_ALIVE && player->systemPower > 0)
	{
		handleKeyboard();

		handleMouse();

		if (!player->target || player->target->health <= 0 || player->target->systemPower <= 0 || targetOutOfRange())
		{
			selectTarget();
		}
	}

	player->angle = ((int)player->angle) % 360;

	if (player->health <= 0 && battle.status == MS_IN_PROGRESS)
	{
		battle.stats[STAT_PLAYER_KILLED]++;
		
		/* the player is known as "Player", so we need to check the craft they were assigned to */
		if (strcmp(game.currentMission->craft, "ATAF") == 0)
		{
			awardTrophy("ATAF_DESTROYED");
		}
		
		if (game.currentMission->challengeData.isChallenge)
		{
			if (!game.currentMission->challengeData.allowPlayerDeath)
			{
				failMission();
			}
		}
		else if (!battle.isEpic)
		{
			failMission();
		}
		else if (player->health == -FPS)
		{
			initPlayerSelect();
		}
	}

	if (battle.status == MS_IN_PROGRESS)
	{
		selectMissionTarget();
	}

	if (dev.playerUnlimitedMissiles)
	{
		player->missiles = 999;
	}
	
	/* really only used in challenge mode */
	if (player->systemPower <= 0 && battle.status == MS_IN_PROGRESS)
	{
		if (game.currentMission->challengeData.isChallenge)
		{
			addHudMessage(colors.red, _("Challenge Failed!"));
			failMission();
		}
	}

	if (battle.boostTimer == (int)BOOST_FINISHED_TIME)
	{
		deactivateBoost();
	}
}

static void rechargeBoostECM(void)
{
	int boostTimer, ecmTimer;
	
	boostTimer = battle.boostTimer;
	battle.boostTimer = MIN(battle.boostTimer + 1, BOOST_RECHARGE_TIME);
	if (boostTimer < BOOST_RECHARGE_TIME && battle.boostTimer == BOOST_RECHARGE_TIME)
	{
		playSound(SND_RECHARGED);
	}
	
	ecmTimer = battle.ecmTimer;
	battle.ecmTimer = MIN(battle.ecmTimer + 1, ECM_RECHARGE_TIME);
	if (ecmTimer < ECM_RECHARGE_TIME && battle.ecmTimer == ECM_RECHARGE_TIME)
	{
		playSound(SND_RECHARGED);
	}
}

static int targetOutOfRange(void)
{
	return (app.gameplay.autoSwitchPlayerTarget && getDistance(player->x, player->y, player->target->x, player->target->y) > SCREEN_WIDTH * 2);
}

static void applyRestrictions(void)
{
	if (game.currentMission->challengeData.noMissiles)
	{
		player->missiles = 0;
	}
	
	if (game.currentMission->challengeData.noBoost)
	{
		battle.boostTimer = 0;
	}
	
	if (game.currentMission->challengeData.noECM)
	{
		battle.ecmTimer = 0;
	}
	
	if (game.currentMission->challengeData.noGuns)
	{
		player->reload = 1;
	}
}

static void handleKeyboard(void)
{
	if (battle.status == MS_IN_PROGRESS)
	{
		if (isControl(CONTROL_BOOST) && player->speed > 0)
		{
			if (battle.boostTimer == BOOST_RECHARGE_TIME)
			{
				playSound(SND_BOOST);

				activateBoost();
			}
			else
			{
				playSound(SND_GUI_DENIED);
			}

			clearControl(CONTROL_BOOST);
		}

		if (isControl(CONTROL_TARGET))
		{
			selectTarget();

			clearControl(CONTROL_TARGET);
		}

		if (isControl(CONTROL_ECM))
		{
			if (battle.ecmTimer == ECM_RECHARGE_TIME)
			{
				playSound(SND_ECM);

				activateECM();
			}
			else
			{
				playSound(SND_GUI_DENIED);
			}

			clearControl(CONTROL_ECM);
		}

		if (isControl(CONTROL_BRAKE))
		{
			applyFighterBrakes();
		}

		if (isControl(CONTROL_GUNS))
		{
			switchGuns();

			clearControl(CONTROL_GUNS);
		}

		if (isControl(CONTROL_RADAR))
		{
			cycleRadarZoom();

			clearControl(CONTROL_RADAR);
		}

		if (isControl(CONTROL_MISSILE))
		{
			preFireMissile();

			clearControl(CONTROL_MISSILE);
		}
	}
	else
	{
		applyFighterBrakes();
	}
}

static void handleMouse(void)
{
	faceMouse();
	
	if (battle.status == MS_IN_PROGRESS)
	{
		if (isControl(CONTROL_FIRE) && !player->reload && player->guns[0].type)
		{
			if (player->selectedGunType != BT_ROCKET)
			{
				fireGuns(player);
			}
			else
			{
				fireRocket(player);
			}
			
			if (battle.hasSuspicionLevel && !battle.numEnemies && !battle.suspicionCoolOff)
			{
				battle.suspicionLevel += (MAX_SUSPICION_LEVEL * 0.05);
			}
		}
		
		if (isControl(CONTROL_ACCELERATE))
		{
			if (battle.boostTimer > BOOST_FINISHED_TIME || game.currentMission->challengeData.noBoost)
			{
				applyFighterThrust();
			}
		}
		
		if (isControl(CONTROL_MISSILE))
		{
			preFireMissile();
			
			app.mouse.button[SDL_BUTTON_MIDDLE] = 0;
		}
		
		if (isControl(CONTROL_GUNS))
		{
			switchGuns();
			
			app.mouse.button[SDL_BUTTON_X1] = 0;
		}
		
		if (isControl(CONTROL_RADAR))
		{
			cycleRadarZoom();
			
			app.mouse.button[SDL_BUTTON_X2] = 0;
		}
	}
}

static void faceMouse(void)
{
	int dir;
	int x, y, wantedAngle;

	x = player->x - battle.camera.x;
	y = player->y - battle.camera.y;
	wantedAngle = getAngle(x, y, app.mouse.x, app.mouse.y);

	wantedAngle %= 360;

	if (fabs(wantedAngle - player->angle) > 2)
	{
		dir = ((int)(wantedAngle - player->angle + 360)) % 360 > 180 ? -1 : 1;

		player->angle += dir * 4;

		player->angle = mod(player->angle, 360);
	}
}

static void preFireMissile(void)
{
	if (player->missiles && player->target)
	{
		if (getDistance(player->x, player->y, player->target->x, player->target->y) <= SCREEN_WIDTH)
		{
			fireMissile(player);
		}
		else
		{
			playSound(SND_GUI_DENIED);
			
			addHudMessage(colors.white, _("Target not in range"));
		}
	}
	else if (!player->missiles)
	{
		addHudMessage(colors.white, _("Out of missiles"));
		
		playSound(SND_NO_MISSILES);
	}
}

void initPlayerSelect(void)
{
	Entity *e;

	memset(&availablePlayerUnits, 0, sizeof(Entity*) * MAX_SELECTABLE_PLAYERS);

	selectedPlayerIndex = 0;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->type == ET_FIGHTER && e->health > 0 && e->side == SIDE_ALLIES && selectedPlayerIndex < MAX_SELECTABLE_PLAYERS)
		{
			availablePlayerUnits[selectedPlayerIndex++] = e;
		}
	}

	if (selectedPlayerIndex > 0)
	{
		battle.playerSelect = 1;
		selectedPlayerIndex = 0;
		memset(&app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
	}
	else
	{
		battle.isEpic = 0;

		failMission();
	}
}

void doPlayerSelect(void)
{
	if (isControl(CONTROL_PREV_FIGHTER))
	{
		selectNewPlayer(-1);
		
		clearControl(CONTROL_PREV_FIGHTER);
	}
	
	if (isControl(CONTROL_NEXT_FIGHTER))
	{
		selectNewPlayer(1);
		
		clearControl(CONTROL_NEXT_FIGHTER);
	}
	
	if (player->health > 0 && isAcceptControl())
	{
		battle.playerSelect = 0;
		
		initPlayer();
		
		resetAcceptControls();
	}
}

static void selectNewPlayer(int dir)
{
	player = NULL;
	
	do
	{
		selectedPlayerIndex += dir;

		selectedPlayerIndex = mod(selectedPlayerIndex, MAX_SELECTABLE_PLAYERS);

		player = availablePlayerUnits[selectedPlayerIndex];
	}
	while (player == NULL);

	battle.camera.x = player->x - (SCREEN_WIDTH / 2);
	battle.camera.y = player->y - (SCREEN_HEIGHT / 2);
}

static void activateBoost(void)
{
	player->dx += sin(TO_RAIDANS(player->angle)) * 10;
	player->dy += -cos(TO_RAIDANS(player->angle)) * 10;
	player->thrust = sqrt((player->dx * player->dx) + (player->dy * player->dy));

	battle.boostTimer = 0;

	battle.stats[STAT_BOOST]++;
}

static void deactivateBoost(void)
{
	float v, thrust;

	thrust = -1;

	while (thrust != player->thrust)
	{
		thrust = player->thrust;

		v = (player->speed / sqrt(player->thrust));
		player->dx = v * player->dx;
		player->dy = v * player->dy;
		player->thrust = sqrt((player->dx * player->dx) + (player->dy * player->dy));
	}
}

static void activateECM(void)
{
	battle.ecmTimer = 0;

	addECMEffect(player);

	battle.stats[STAT_ECM]++;
	
	if (battle.hasSuspicionLevel && !battle.numEnemies)
	{
		battle.suspicionLevel += (MAX_SUSPICION_LEVEL * 0.25);
	}
}

static void switchGuns(void)
{
	int i;

	i = player->selectedGunType;

	if (!player->combinedGuns)
	{
		do
		{
			i = (i + 1) % BT_MAX;
		}
		while (!availableGuns[i]);
	}

	if (player->selectedGunType != i)
	{
		playSound(SND_SELECT_WEAPON);
		player->selectedGunType = i;
	}
}

static void selectTarget(void)
{
	unsigned int closest = MAX_TARGET_RANGE;
	unsigned int dist = MAX_TARGET_RANGE;
	int i, total;
	Entity *e, *near;
	Entity *targets[MAX_SELECTABLE_TARGETS];

	i = 0;
	near = NULL;
	memset(targets, 0, sizeof(Entity*) * MAX_SELECTABLE_TARGETS);
	
	if (player->target && (!player->target->health || !player->target->systemPower))
	{
		player->target = NULL;
	}
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e != player && (e->flags & EF_TAKES_DAMAGE) && (!(e->flags & EF_NO_PLAYER_TARGET)) && e->side != player->side && e->alive == ALIVE_ALIVE && e->systemPower > 0 && i < MAX_SELECTABLE_TARGETS)
		{
			dist = getDistance(player->x, player->y, e->x, e->y);
			
			if (dist < closest)
			{
				near = e;
				closest = dist;
			}

			if (collision(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, e->x - battle.camera.x - (e->w / 2), e->y - battle.camera.y - (e->h / 2), e->w, e->h))
			{
				targets[i++] = e;
			}
			else if (e == player->target)
			{
				player->target = NULL;
			}
		}
	}

	total = i;

	for (i = 0 ; i < total ; i++)
	{
		if (targets[i] == player->target)
		{
			if (i + 1 < MAX_SELECTABLE_TARGETS && targets[i + 1])
			{
				player->target = targets[i + 1];
				return;
			}
			else
			{
				player->target = targets[0];
			}
		}
	}

	if (!player->target || !targets[0])
	{
		player->target = near;
	}
}

static void selectMissionTarget(void)
{
	unsigned int closest = MAX_TARGET_RANGE;
	unsigned int dist = MAX_TARGET_RANGE;
	Entity *e;

	battle.missionTarget = NULL;

	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->active && e->flags & EF_MISSION_TARGET && e->alive == ALIVE_ALIVE)
		{
			dist = getDistance(player->x, player->y, e->x, e->y);

			if (battle.missionTarget == NULL)
			{
				battle.missionTarget = e;
				closest = dist;
			}
			else if (battle.missionTarget->type == ET_WAYPOINT && e->type != ET_WAYPOINT)
			{
				battle.missionTarget = e;
			}
			else if (battle.missionTarget->type != ET_WAYPOINT)
			{
				if (isPriorityMissionTarget(e, dist, closest))
				{
					battle.missionTarget = e;
					closest = dist;
				}
			}
			else if (battle.missionTarget->type == ET_WAYPOINT && e->type == ET_WAYPOINT && e->id == battle.missionTarget->id)
			{
				battle.missionTarget = e;
			}
		}
	}
}

static int isPriorityMissionTarget(Entity *e, int dist, int closest)
{
	/* battle.missionTarget is secondary, e is not */
	if ((battle.missionTarget->flags & EF_SECONDARY_TARGET) > (e->flags & EF_SECONDARY_TARGET))
	{
		return 1;
	}
	
	/* battle.missionTarget is not secondary, e is */
	if ((battle.missionTarget->flags & EF_SECONDARY_TARGET) < (e->flags & EF_SECONDARY_TARGET))
	{
		return 0;
	}
	
	/* normal distance check */
	return dist < closest;
}

void setInitialPlayerAngle(void)
{
	Entity *e;
	
	selectMissionTarget();
	
	if (battle.missionTarget)
	{
		player->angle = getAngle(player->x, player->y, battle.missionTarget->x, battle.missionTarget->y);
	}
	else
	{
		selectTarget();
		
		if (player->target)
		{
			player->angle = getAngle(player->x, player->y, player->target->x, player->target->y);
		}
		else
		{
			if (battle.jumpgate)
			{
				player->angle = getAngle(player->x, player->y, battle.jumpgate->x, battle.jumpgate->y);
			}
		}
	}
	
	for (e = battle.entityHead.next ; e != NULL ; e = e->next)
	{
		if (e->side == player->side)
		{
			e->angle = player->angle;
		}
	}
}

static void cycleRadarZoom(void)
{
	battle.radarRange = (battle.radarRange + 1) % 3;
	
	playSound(SND_ZOOM);
}

int playerHasGun(int type)
{
	return availableGuns[type];
}

void loadPlayer(cJSON *node)
{
	char *type;
	int side, addFlags;
	long flags;

	type = cJSON_GetObjectItem(node, "type")->valuestring;
	side = lookup(cJSON_GetObjectItem(node, "side")->valuestring);
	flags = -1;

	player = spawnFighter(type, 0, 0, side);
	player->x = BATTLE_AREA_WIDTH / 2;
	player->y = BATTLE_AREA_HEIGHT / 2;

	if (cJSON_GetObjectItem(node, "x"))
	{
		player->x = (cJSON_GetObjectItem(node, "x")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_WIDTH;
		player->y = (cJSON_GetObjectItem(node, "y")->valuedouble / BATTLE_AREA_CELLS) * BATTLE_AREA_HEIGHT;
	}
	
	if (cJSON_GetObjectItem(node, "flags"))
	{
		flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, &addFlags);
	}
	
	if (flags != -1)
	{
		if (addFlags)
		{
			player->flags |= flags;
		}
		else
		{
			player->flags = flags;

			SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Flags for Player replaced");
		}
	}

	if (strcmp(type, "Tug") == 0)
	{
		battle.stats[STAT_TUG]++;
	}

	if (strcmp(type, "Shuttle") == 0)
	{
		battle.stats[STAT_SHUTTLE]++;
	}
}
