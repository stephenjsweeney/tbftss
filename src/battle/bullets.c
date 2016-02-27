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

#include "bullets.h"

static void huntTarget(Bullet *b);
static void checkCollisions(Bullet *b);
static void resizeDrawList(void);

static Bullet bulletDef[BT_MAX];
static Bullet **bulletsToDraw;
static int incomingMissile;
static int drawCapacity;

void initBullets(void)
{
	incomingMissile = 0;

	drawCapacity = INITIAL_BULLET_DRAW_CAPACITY;

	bulletsToDraw = malloc(sizeof(Bullet*) * drawCapacity);
	memset(bulletsToDraw, 0, sizeof(Bullet*) * drawCapacity);
}

void initBulletDefs(void)
{
	cJSON *root, *node;
	char *text;
	int type;
	Bullet *def;

	memset(&bulletDef, 0, sizeof(Bullet) * BT_MAX);

	text = readFile(getFileLocation("data/battle/bullets.json"));

	root = cJSON_Parse(text);

	for (node = root->child ; node != NULL ; node = node->next)
	{
		type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);

		def = &bulletDef[type];
		def->type = type;
		def->damage = cJSON_GetObjectItem(node, "damage")->valueint;
		def->texture = getTexture(cJSON_GetObjectItem(node, "texture")->valuestring);
		def->sound = lookup(cJSON_GetObjectItem(node, "sound")->valuestring);
		def->flags = flagsToLong(cJSON_GetObjectItem(node, "flags")->valuestring, NULL);

		SDL_QueryTexture(def->texture, NULL, NULL, &def->w, &def->h);
	}

	cJSON_Delete(root);
	free(text);
}

void doBullets(void)
{
	int i = 0;
	Bullet *b;
	Bullet *prev = &battle.bulletHead;

	incomingMissile = 0;

	memset(bulletsToDraw, 0, sizeof(Bullet*) * drawCapacity);

	for (b = battle.bulletHead.next ; b != NULL ; b = b->next)
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

			huntTarget(b);

			if (b->target == player && player != NULL && player->health > 0)
			{
				incomingMissile = 1;
			}
		}

		checkCollisions(b);

		if (--b->life <= 0)
		{
			if (player != NULL && player->alive == ALIVE_ALIVE && b->type == BT_MISSILE && b->damage > 0 && b->target == player)
			{
				battle.stats[STAT_MISSILES_EVADED]++;
			}

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
			if (collision(b->x - (b->w / 2) - battle.camera.x, b->y - (b->h / 2) - battle.camera.y, b->w * 2, b->h * 2, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT))
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
	int i, n;
	Bullet **bullets;

	n = drawCapacity + INITIAL_BULLET_DRAW_CAPACITY;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Resizing bullet draw capacity: %d -> %d\n", drawCapacity, n);

	bullets = malloc(sizeof(Bullet*) * n);
	memset(bullets, 0, sizeof(Bullet*) * n);

	for (i = 0 ; i < drawCapacity ; i++)
	{
		bullets[i] = bulletsToDraw[i];
	}

	free(bulletsToDraw);

	bulletsToDraw = bullets;
	drawCapacity = n;
}

static void checkCollisions(Bullet *b)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsWithin(b->x - (b->w / 2), b->y - (b->h / 2), b->w, b->h, NULL);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->flags & EF_TAKES_DAMAGE)
		{
			if (b->owner->owner != NULL && b->owner->owner == e->owner)
			{
				continue;
			}

			if (b->owner != e && e->health > 0 && collision(b->x - b->w / 2, b->y - b->h / 2, b->w, b->h, e->x - e->w / 2, e->y - e->h / 2, e->w, e->h))
			{
				if (b->owner->side == e->side)
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
				}

				if (!(e->flags & EF_IMMORTAL))
				{
					damageFighter(e, b->damage, b->flags);
				}

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

				/* assuming that health <= 0 will always mean killed */
				if (e->health <= 0 && b->owner == player)
				{
					battle.stats[STAT_ENEMIES_KILLED_PLAYER]++;
					battle.stats[STAT_EPIC_KILL_STREAK]++;
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

void drawBullets(void)
{
	int i;
	Bullet *b;

	for (i = 0, b = bulletsToDraw[i] ; b != NULL ; b = bulletsToDraw[++i])
	{
		blitRotated(b->texture, b->x - battle.camera.x, b->y - battle.camera.y, b->angle);
	}

	if (incomingMissile && battle.stats[STAT_TIME] % FPS < 40)
	{
		drawText(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60, 18, TA_CENTER, colors.red, _("WARNING: INCOMING MISSILE!"));
	}
}

static void faceTarget(Bullet *b)
{
	int dir;
	int wantedAngle = getAngle(b->x, b->y, b->target->x, b->target->y);

	wantedAngle %= 360;

	if (fabs(wantedAngle - b->angle) > TURN_THRESHOLD)
	{
		dir = (wantedAngle - b->angle + 360) % 360 > 180 ? -1 : 1;

		b->angle += dir * TURN_SPEED;

		b->angle = mod(b->angle, 360);

		/* lower your speed while you're not at the correct angle */
		b->dx *= 0.38;
		b->dy *= 0.38;
	}
}

static void applyMissileThrust(Bullet *b)
{
	int maxSpeed;
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
		b->target = NULL;
	}
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
	int i;
	float x, y;
	float c, s;

	for (i = 0 ; i < MAX_FIGHTER_GUNS ; i++)
	{
		if (owner->guns[i].type == owner->selectedGunType || (owner->guns[i].type != BT_NONE && owner->combinedGuns))
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

	playBattleSound(b->sound, owner->x, owner->y);
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

	b->life = FPS * 30;

	owner->missiles--;

	if (owner == player)
	{
		battle.stats[STAT_MISSILES_FIRED]++;
	}

	playBattleSound(b->sound, owner->x, owner->y);

	if (owner->target == player)
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
