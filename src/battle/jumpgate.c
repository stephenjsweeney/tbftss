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

#include "jumpgate.h"

static void think(void);
static void draw(void);
static void handleFleeingEntities(void);
static void addEscapeEffect(Entity *ent);
static void addNodes(Entity *jumpgate, long flags);
static void nodeDie(void);

static SDL_Texture *portal;
static float portalAngle;

Entity *spawnJumpgate(int side, long flags)
{
	Entity *jumpgate;
	
	if (battle.jumpgate)
	{
		printf("ERROR: Only one jumpgate is allowed\n");
		exit(1);
	}
	
	jumpgate = spawnEntity();
	jumpgate->type = ET_JUMPGATE;
	jumpgate->health = jumpgate->maxHealth = 1;
	jumpgate->texture = getTexture("gfx/entities/jumpgate.png");
	jumpgate->action = think;
	jumpgate->draw = draw;
	jumpgate->side = side;
	jumpgate->flags = EF_NO_MT_BOX+EF_IMMORTAL+EF_AI_IGNORE+EF_NON_SOLID+EF_NO_HEALTH_BAR;
	
	if (flags != -1 && flags & EF_DISABLED)
	{
		jumpgate->flags |= EF_DISABLED;
	}
	
	addNodes(jumpgate, flags);

	portal = getTexture("gfx/entities/portal.png");
	portalAngle = 0;
	
	SDL_QueryTexture(jumpgate->texture, NULL, NULL, &jumpgate->w, &jumpgate->h);
	
	battle.jumpgate = jumpgate;

	return jumpgate;
}

static void addNodes(Entity *jumpgate, long flags)
{
	Entity *node;
	SDL_Texture *nodeTexture;
	int i;
	
	nodeTexture = getTexture("gfx/entities/jumpgateNode.png");
	
	for (i = 0 ; i < 360 ; i += 36)
	{
		node = spawnEntity();
		STRNCPY(node->name, _("Jumpgate System Node"), MAX_NAME_LENGTH);
		node->health = node->maxHealth = 25;
		node->type = ET_COMPONENT;
		node->offsetX = sin(TO_RAIDANS(i)) * 215;
		node->offsetY = -cos(TO_RAIDANS(i)) * 215;
		node->owner = jumpgate;
		node->side = jumpgate->side;
		node->texture = nodeTexture;
		node->flags = EF_TAKES_DAMAGE+EF_AI_IGNORE;
		node->die = nodeDie;
		SDL_QueryTexture(node->texture, NULL, NULL, &node->w, &node->h);
		
		if (jumpgate->side == SIDE_NONE)
		{
			node->flags |= EF_NO_HEALTH_BAR;
		}
		
		if (flags != -1)
		{
			node->flags = flags;
		}
		
		jumpgate->maxHealth++;
	}
	
	jumpgate->health = jumpgate->maxHealth;
}

static void nodeDie(void)
{
	self->alive = ALIVE_DEAD;
	addSmallExplosion();
	playBattleSound(SND_EXPLOSION_1 + rand() % 4, self->x, self->y);
	addDebris(self->x, self->y, 3 + rand() % 4);

	if (--battle.jumpgate->health == 1)
	{
		battle.jumpgate->flags |= EF_DISABLED;
		
		updateObjective("Jumpgate", TT_DESTROY);
		updateCondition("Jumpgate", TT_DESTROY);
	}

	runScriptFunction("JUMPGATE_HEALTH %d", battle.jumpgate->health);
}

int jumpgateEnabled(void)
{
	return (battle.jumpgate && (!(battle.jumpgate->flags & EF_DISABLED)));
}

void activateJumpgate(int activate)
{
	Entity *e;
	
	if (battle.jumpgate && battle.jumpgate->health > 1)
	{
		for (e = battle.entityHead.next ; e != NULL ; e = e->next)
		{
			if (e == battle.jumpgate || e->owner == battle.jumpgate)
			{
				if (activate)
				{
					e->flags &= ~EF_DISABLED;
				}
				else
				{
					e->flags |= EF_DISABLED;
				}
			}
		}
	}
}

static void think(void)
{
	self->thinkTime = 4;

	self->angle += 0.1;
	
	if (self->angle >= 360)
	{
		self->angle -= 360;
	}
	
	if (jumpgateEnabled())
	{
		handleFleeingEntities();
	}
	
	portalAngle += 2;
	
	if (portalAngle >= 360)
	{
		portalAngle -= 360;
	}
}

static void handleFleeingEntities(void)
{
	Entity *e, **candidates;
	int i;

	candidates = getAllEntsInRadius(self->x, self->y, ESCAPE_DISTANCE * 2, self);

	for (i = 0, e = candidates[i] ; e != NULL ; e = candidates[++i])
	{
		if (e->health > 0 && (e->flags & EF_RETREATING) && getDistance(self->x, self->y, e->x, e->y) <= ESCAPE_DISTANCE)
		{
			e->alive = ALIVE_ESCAPED;

			addEscapeEffect(e);

			playBattleSound(SND_JUMP, e->x, e->y);
		}
	}
}

static void addEscapeEffect(Entity *ent)
{
	Effect *e;
	int i, n, speed;

	n = ent->w * ent->h;

	for (i = 0 ; i < n ; i++)
	{
		e = malloc(sizeof(Effect));
		memset(e, 0, sizeof(Effect));
		battle.effectTail->next = e;
		battle.effectTail = e;

		speed = 50 + rand() % 50;

		e->type = EFFECT_POINT;
		e->x = ent->x + (rand() % ent->w) - (rand() % ent->w);
		e->y = ent->y + (rand() % ent->h) - (rand() % ent->w);

		e->dx = self->x - e->x;
		e->dx /= speed;

		e->dy = self->y - e->y;
		e->dy /= speed;

		e->r = e->g = e->b = e->a = (rand() % 255);
		e->health = speed;
	}
}

static void draw(void)
{
	if (jumpgateEnabled())
	{
		blitRotated(portal, self->x - battle.camera.x, self->y - battle.camera.y, portalAngle);
	}

	blitRotated(self->texture, self->x - battle.camera.x, self->y - battle.camera.y, self->angle);
}
