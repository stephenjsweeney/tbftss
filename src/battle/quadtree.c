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

#include "quadtree.h"

static Entity *candidates[QT_MAX_CANDIDATES];
static int cIndex;
static int memory;

static int getIndex(Quadtree *root, int x, int y, int w, int h);
static void removeEntity(Entity *e, Quadtree *root);
static void addCandidate(Entity *e);
static int candidatesComparator(const void *a, const void *b);
static void getAllEntsWithinNode(int x, int y, int w, int h, Entity *ignore, Quadtree *root);
static void destroyQuadtreeNode(Quadtree *root);

void initQuadtree(Quadtree *root)
{
	Quadtree *node;
	int i, w, h;
	
	if (root->depth == 0)
	{
		memory = 0;
	}
	
	memory += sizeof(Quadtree);
	
	w = root->w / 2;
	h = root->h / 2;
	
	if (root->depth + 1 < QT_MAX_DEPTH)
	{
		for (i = 0 ; i < 4 ; i++)
		{
			node = malloc(sizeof(Quadtree));
			memset(node, 0, sizeof(Quadtree));
			root->node[i] = node;
			
			node->depth = root->depth + 1;
			
			if (i == 0)
			{
				node->x = root->x;
				node->y = root->y;
				node->w = w;
				node->h = h;
			}
			else if (i == 1)
			{
				node->x = root->x + w;
				node->y = root->y;
				node->w = w;
				node->h = h;
			}
			else if (i == 2)
			{
				node->x = root->x;
				node->y = root->y + h;
				node->w = w;
				node->h = h;
			}
			else
			{
				node->x = root->x + w;
				node->y = root->y + h;
				node->w = w;
				node->h = h;
			}
			
			initQuadtree(node);
		}
	}
	
	if (root->depth == 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Quadtree memory footprint: %d bytes", memory);
	}
}

void addToQuadtree(Entity *e, Quadtree *root)
{
	int index;
	
	if (root->node[0])
	{
		index = getIndex(root, e->x - (e->w / 2), e->y - (e->h / 2), e->w, e->h);
		
		if (index != -1)
		{
			addToQuadtree(e, root->node[index]);
			return;
		}
	}
	
	if (root->numEnts < QT_MAX_ENTS)
	{
		root->ents[root->numEnts++] = e;
		return;
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Couldn't add %s to quadtree - out of node ent space", e->name);
}

static int getIndex(Quadtree *root, int x, int y, int w, int h)
{
	int index = -1;

	int verticalMidpoint = root->x + (root->w / 2);
	int horizontalMidpoint = root->y + (root->h / 2);
	int topQuadrant = (y < horizontalMidpoint && y + h < horizontalMidpoint);
	int bottomQuadrant = (y > horizontalMidpoint);

	if (x < verticalMidpoint && x + w < verticalMidpoint)
	{
		if (topQuadrant)
		{
			index = 1;
		}
		else if (bottomQuadrant)
		{
			index = 2;
		}
	}
	else if (x > verticalMidpoint)
	{
		if (topQuadrant)
		{
			index = 0;
		}
		else if (bottomQuadrant)
		{
			index = 3;
		}
	}

	return index;
}

void removeFromQuadtree(Entity *e, Quadtree *root)
{
	int index, i;
	
	index = getIndex(root, e->x - (e->w / 2), e->y - (e->h / 2), e->w, e->h);

	removeEntity(e, root);

	if (root->node[0])
	{
		if (index != -1)
		{
			removeFromQuadtree(e, root->node[index]);
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
				removeFromQuadtree(e, root->node[i]);
			}
		}
	}
}

static void removeEntity(Entity *e, Quadtree *root)
{
	int i, n;
	
	n = root->numEnts;
	
	for (i = 0 ; i < QT_MAX_ENTS ; i++)
	{
		if (root->ents[i] == e)
		{
			root->ents[i] = NULL;
			root->numEnts--;
		}
	}
	
	qsort(root->ents, n, sizeof(Entity*), candidatesComparator);
}

Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore)
{
	cIndex = 0;
	memset(candidates, 0, sizeof(Entity*) * QT_MAX_CANDIDATES);
	
	getAllEntsWithinNode(x, y, w, h, ignore, &battle.quadtree);
	
	/* NULL terminate, to allow us to loop through them more easily */
	candidates[QT_MAX_CANDIDATES - 1] = NULL;
	
	return candidates;
}

static void getAllEntsWithinNode(int x, int y, int w, int h, Entity *ignore, Quadtree *root)
{
	Quadtree *node;
	int index, i;
	
	for (i = 0 ; i < root->numEnts ; i++)
	{
		addCandidate(root->ents[i]);
	}
	
	index = getIndex(root, x, y, w, h);

	if (root->node[0])
	{
		if (index != -1)
		{
			node = root->node[index];
			
			getAllEntsWithinNode(node->x, node->y, node->w, node->h, ignore, node);
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
				node = root->node[i];
			
				getAllEntsWithinNode(node->x, node->y, node->w, node->h, ignore, node);
			}
		}
	}
}

static void addCandidate(Entity *e)
{
	int i;
	
	for (i = 0 ; i <= cIndex ; i++)
	{
		/* already added */
		if (candidates[i] == e)
		{
			return;
		}
		
		if (!candidates[i])
		{
			candidates[cIndex++] = e;
			return;
		}
	}
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Out of quadtree candidate space");
	exit(1);
}

void destroyQuadtree(void)
{
	destroyQuadtreeNode(&battle.quadtree);
}

static void destroyQuadtreeNode(Quadtree *root)
{
	int i;
	
	if (root->node[0])
	{
		for (i = 0 ; i < 4 ; i++)
		{
			destroyQuadtreeNode(root->node[i]);
			
			free(root->node[i]);
			
			root->node[i] = NULL;
		}
	}
}

static int candidatesComparator(const void *a, const void *b)
{
	Entity *e1 = *((Entity**)a);
	Entity *e2 = *((Entity**)b);
	
	if (!e1)
    {
        return 1;
    }
    else if (!e2)
	{
		return -1;
	}
    else
	{
		return 0;
	}
}
