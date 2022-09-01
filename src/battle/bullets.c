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

#include "../battle/effects.h"
#include "../battle/fighters.h"
#include "../battle/quadtree.h"
#include "../game/trophies.h"
#include "../json/cJSON.h"
#include "../system/atlas.h"
#include "../system/draw.h"
#include "../system/io.h"
#include "../system/lookup.h"
#include "../system/sound.h"
#include "../system/util.h"
#include "bullets.h"

#define INITIAL_BULLET_DRAW_CAPACITY 32
#define MISSILE_LIFE                 (FPS * 30)
#define TURN_SPEED                   2
#define TURN_THRESHOLD               3

extern App     app;
extern Battle  battle;
extern Entity *player;

static void huntTarget(Bullet *b);
static void checkCollisions(Bullet *b);
static void resizeDrawList(void);
static void selectNewTarget(Bullet *b);
static void doBulletHitEffect(Bullet *b);

static Bullet   bulletDef[BT_MAX];
static Bullet **bulletsToDraw;
static int      drawCapacity;

void initBullets(void)
{
	drawCapacity = INITIAL_BULLET_DRAW_CAPACITY;

	bulletsToDraw = malloc(sizeof(Bullet *) * drawCapacity);
	memset(bulletsToDraw, 0, sizeof(Bullet *) * drawCapacity);
}

void initBulletDefs(void)
{
	cJSON  *root, *node;
	char   *text;
	int     type;
	Bullet *def;

	memset(&bulletDef, 0, sizeof(Bullet) * BT_MAX);

	text = readFile("data/battle/bullets.json");

	root = cJSON_Parse(text);

	for (node = root->child; node != NULL; node = node->next)
	{
		type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);

		def = &bulletDef[type];
		def->type = type;
		def->damage = cJSON_GetObjectItem(node, "damage")->valueint;
		def->texture = getAtlasImage(cJSON_GetObjectItem(node, "texture")->valuestring);
		def->sound = lookup(cJSON_GetObjectItem(node, "sound")->valuestring);
		def->flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, NULL);

		def->w = def->texture->rect.w;
		def->h = def->texture->rect.h;
	}

	cJSON_Delete(root);
	free(text);
}

void doBullets(void)
{
	int     i = 0;
	Bullet *b;
	Bullet *prev = &battle.bulletHead;

	battle.incomingMissile = 0;

	memset(bulletsToDraw, 0, sizeof(Bullet *) * drawCapacity);

	for (b = battle.bulletHead.next; b != NULL; b = b->next)
	{
		b->x += b->dx;
		b->y += b->dy;

		if (b->type == BT_ROCKET)
		{
			addMissileEngineEffect(b);
		}
		else if (b->type == BT_MISSILE)
		{
			addMissileEngineEffect(b);

			if (b->life < MISSILE_LIFE - (FPS / 4))
			{
				huntTarget(b);
			}

			if (b->target == player && player->alive == ALIVE_ALIVE && player->health > 0)
			{
				battle.incomingMissile = 1;
			}
		}

		checkCollisions(b);

		if (--b->life <= 0)
		{
			if (b == battle.bulletTail)
			{
				battle.bulletTail = prev;
			}

			prev->next = b->next;
			free(b);
			b = prev;
		}
		else
		{
			if (isOnBattleScreen(b->x, b->y, b->w, b->h))
			{
				bulletsToDraw[i++] = b;

				if (i == drawCapacity)
				{
					resizeDrawList();
				}
			}
		}

		prev = b;
	}
}

static void resizeDrawList(void)
{
	int n;

	n = drawCapacity + INITIAL_BULLET_DRAW_CAPACITY;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Resizing bullet draw capacity: %d -> %d", drawCapacity, n);

	bulletsToDraw = resize(bulletsToDraw, sizeof(Bullet *) * drawCapacity, sizeof(Bullet *) * n);

	drawCapacity = n;
}

static void checkCollisions(Bullet *b)
{
	Entity *e, **candidates;
	int     i;

	candidates = getAllEntsWithin(b->x - (b->w / 2), b->y - (b->h / 2), b->w, b->h, NULL);

	for (i = 0, e = candidates[i]; e != NULL; e = candidates[++i])
	{
		if (e->flags & EF_TAKES_DAMAGE)
		{
			if (b->owner->owner != NULL && b->owner->owner == e->owner)
			{
				continue;
			}

			if (b->owner != e && e->health > 0 && collision(b->x - b->w / 2, b->y - b->h / 2, b->w, b->h, e->x - e->w / 2, e->y - e->h / 2, e->w, e->h))
			{
				if (b->owner->side == e->side && !app.gameplay.friendlyFire && (!(e->flags & EF_DISABLED)) && e->type != ET_MINE)
				{
					b->damage = 0;
				}
				else if (b->owner == player)
				{
					if (b->type != BT_ROCKET)
					{
						battle.stats[STAT_SHOTS_HIT]++;
					}
					else
					{
						battle.stats[STAT_ROCKETS_HIT]++;
					}

					if (battle.hasSuspicionLevel)
					{
						if (e->aiFlags & (AIF_AVOIDS_COMBAT | AIF_DEFENSIVE))
						{
							battle.suspicionLevel -= (MAX_SUSPICION_LEVEL * 0.1);
						}
						else
						{
							battle.suspicionLevel -= (MAX_SUSPICION_LEVEL * 0.001);
						}
					}
				}

				if (e->flags & EF_IMMORTAL)
				{
					b->damage = 0;
				}

				damageFighter(e, b->damage, b->flags);

				doBulletHitEffect(b);

				b->life = 0;
				b->damage = 0;

				if (b->flags & BF_EXPLODES)
				{
					addMissileExplosion(b);
					playBattleSound(SND_EXPLOSION_1, b->x, b->y);

					if (e == player)
					{
						battle.stats[STAT_MISSILES_STRUCK]++;
					}
				}

				/* missile was targetting player, but hit something else */
				if (b->type == BT_MISSILE && b->target == player && e != player)
				{
					battle.stats[STAT_MISSILES_EVADED]++;
				}

				if (b->type == BT_MISSILE && b->target != e)
				{
					if (e == player)
					{
						awardTrophy("TEAM_PLAYER");
					}
					else if (b->owner == player && (e->aiFlags & AIF_MOVES_TO_LEADER) && (b->target->flags & EF_AI_LEADER))
					{
						awardTrophy("BODYGUARD");
					}
				}

				/* assuming that health <= 0 will always mean killed */
				if (e->health <= 0)
				{
					e->killedBy = b->owner;

					if (e == player)
					{
						battle.lastKilledPlayer = b->owner;
					}

					if (battle.isEpic && b->owner == player && e == battle.lastKilledPlayer)
					{
						awardTrophy("REVENGE");
					}
				}

				if (b->owner == player && b->type == BT_MISSILE)
				{
					battle.stats[STAT_MISSILES_HIT]++;
				}

				return;
			}
		}
	}
}

static void doBulletHitEffect(Bullet *b)
{
	switch (b->type)
	{
		case BT_PARTICLE:
			addBulletHitEffect(b->x, b->y, 255, 0, 255);
			break;

		case BT_PLASMA:
			addBulletHitEffect(b->x, b->y, 0, 255, 0);
			break;

		case BT_LASER:
			addBulletHitEffect(b->x, b->y, 255, 0, 0);
			break;

		case BT_MAG:
			addBulletHitEffect(b->x, b->y, 196, 196, 255);
			break;

		default:
			addBulletHitEffect(b->x, b->y, 255, 255, 255);
			break;
	}
}

void drawBullets(void)
{
	int     i;
	Bullet *b;

	setAtlasColor(255, 255, 255, 255);

	for (i = 0, b = bulletsToDraw[i]; b != NULL; b = bulletsToDraw[++i])
	{
		blitRotated(b->texture, b->x - battle.camera.x, b->y - battle.camera.y, b->angle);
	}
}

static void faceTarget(Bullet *b)
{
	int dir, wantedAngle, dist;

	wantedAngle = (int)getAngle(b->x, b->y, b->target->x, b->target->y) % 360;

	if (abs(wantedAngle - b->angle) > TURN_THRESHOLD)
	{
		dir = (wantedAngle - b->angle + 360) % 360 > 180 ? -1 : 1;

		b->angle += dir * TURN_SPEED;

		dist = getDistance(b->x, b->y, b->target->x, b->target->y);

		if (dist < 250)
		{
			dist = 250 - dist;

			while (dist > 0)
			{
				b->angle += dir;

				dist -= 50;
			}
		}

		b->angle = mod(b->angle, 360);

		b->dx *= 0.5;
		b->dy *= 0.5;
	}
}

static void applyMissileThrust(Bullet *b)
{
	int   maxSpeed;
	float v, thrust;

	b->dx += sin(TO_RAIDANS(b->angle));
	b->dy += -cos(TO_RAIDANS(b->angle));

	maxSpeed = MAX(MIN(b->target->speed + 1, 999), 3);

	thrust = sqrt((b->dx * b->dx) + (b->dy * b->dy));

	if (thrust > maxSpeed)
	{
		v = (maxSpeed / sqrt(thrust));
		b->dx = v * b->dx;
		b->dy = v * b->dy;
	}
}

static void huntTarget(Bullet *b)
{
	if (b->target != NULL && b->target->health > 0)
	{
		faceTarget(b);

		applyMissileThrust(b);

		if (b->target == player && battle.ecmTimer < FPS)
		{
			b->life = 0;
			addMissileExplosion(b);
			playBattleSound(SND_EXPLOSION_1, b->x, b->y);
		}
	}
	else
	{
		selectNewTarget(b);
	}
}

static void selectNewTarget(Bullet *b)
{
	int     i;
	Entity *e, **candidates;

	if (app.gameplay.missileReTarget)
	{
		b->target = NULL;

		candidates = getAllEntsInRadius(b->x, b->y, SCREEN_HEIGHT, NULL);

		for (i = 0, e = candidates[i]; e != NULL; e = candidates[++i])
		{
			if (e->type == ET_FIGHTER && e->side != b->owner->side && e->health > 0)
			{
				b->target = e;

				if (b->target == player)
				{
					playSound(SND_INCOMING);
				}

				return;
			}
		}
	}

	/* no target, just explode */
	b->life = 0;
	addMissileExplosion(b);
	playBattleSound(SND_EXPLOSION_1, b->x, b->y);
}

static Bullet *createBullet(int type, int x, int y, Entity *owner)
{
	Bullet *b;

	b = malloc(sizeof(Bullet));
	memset(b, 0, sizeof(Bullet));
	battle.bulletTail->next = b;
	battle.bulletTail = b;

	memcpy(b, &bulletDef[type], sizeof(Bullet));

	b->next = NULL;

	b->x = x;
	b->y = y;
	b->dx += sin(TO_RAIDANS(owner->angle)) * 16;
	b->dy += -cos(TO_RAIDANS(owner->angle)) * 16;
	b->life = FPS * 2;
	b->angle = owner->angle;
	b->owner = owner;
	b->target = owner->target;

	return b;
}

void fireGuns(Entity *owner)
{
	Bullet *b;
	int     i;
	float   x, y;
	float   c, s;

	b = NULL;

	for (i = 0; i < MAX_FIGHTER_GUNS; i++)
	{
		if (owner->guns[i].type != BT_NONE && (owner->guns[i].type == owner->selectedGunType || owner->combinedGuns))
		{
			s = sin(TO_RAIDANS(owner->angle));
			c = cos(TO_RAIDANS(owner->angle));

			x = (owner->guns[i].x * c) - (owner->guns[i].y * s);
			y = (owner->guns[i].x * s) + (owner->guns[i].y * c);

			x += owner->x;
			y += owner->y;

			b = createBullet(owner->guns[i].type, x, y, owner);

			if (owner == player)
			{
				battle.stats[STAT_SHOTS_FIRED]++;
			}
		}
	}

	owner->reload = owner->reloadTime;

	if (b)
	{
		playBattleSound(b->sound, owner->x, owner->y);
	}
}

void fireRocket(Entity *owner)
{
	Bullet *b;

	b = createBullet(BT_ROCKET, owner->x, owner->y, owner);

	playBattleSound(b->sound, owner->x, owner->y);

	owner->reload = FPS;

	if (owner == player)
	{
		battle.stats[STAT_ROCKETS_FIRED]++;
	}
}

void fireMissile(Entity *owner)
{
	Bullet *b;

	b = createBullet(BT_MISSILE, owner->x, owner->y, owner);

	b->dx *= 0.5;
	b->dy *= 0.5;

	b->life = MISSILE_LIFE;

	owner->missiles--;

	if (owner == player)
	{
		battle.stats[STAT_MISSILES_FIRED]++;
	}

	playBattleSound(b->sound, owner->x, owner->y);

	if (b->target == player)
	{
		playSound(SND_INCOMING);
	}
}

void destroyBulletDefs(void)
{
}

void destroyBullets(void)
{
	free(bulletsToDraw);

	bulletsToDraw = NULL;
}
