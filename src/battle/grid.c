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

#include "grid.h"

static Entity *candidates[MAX_GRID_CANDIDATES];
static void addCandidate(Entity *e);

void addToGrid(Entity *e)
{
	GridCell *cell, *prev;
	int x, y, x1, y1, x2, y2;
	
	x1 = (e->x - e->w / 2) / GRID_CELL_WIDTH;
	y1 = (e->y - e->h / 2) / GRID_CELL_HEIGHT;
	x2 = (e->x + e->w / 2) / GRID_CELL_WIDTH;
	y2 = (e->y + e->h / 2) / GRID_CELL_HEIGHT;
	
	for (x = x1 ; x <= x2 ; x++)
	{
		for (y = y1 ; y <= y2 ; y++)
		{
			if (x >= 0 && y >= 0 && x < GRID_SIZE && y < GRID_SIZE)
			{
				prev = &battle.grid[x][y];
				
				for (cell = battle.grid[x][y].next ; cell != NULL ; cell = cell->next)
				{
					prev = cell;
				}
				
				cell = malloc(sizeof(GridCell));
				memset(cell, 0, sizeof(GridCell));
				prev->next = cell;
				
				cell->entity = e;
			}
		}
	}
}

void removeFromGrid(Entity *e)
{
	GridCell *cell, *prev;
	int x, y, x1, y1, x2, y2;
	
	x1 = (e->x - e->w / 2) / GRID_CELL_WIDTH;
	y1 = (e->y - e->h / 2) / GRID_CELL_HEIGHT;
	x2 = (e->x + e->w / 2) / GRID_CELL_WIDTH;
	y2 = (e->y + e->h / 2) / GRID_CELL_HEIGHT;
	
	for (x = x1 ; x <= x2 ; x++)
	{
		for (y = y1 ; y <= y2 ; y++)
		{
			if (x >= 0 && y >= 0 && x < GRID_SIZE && y < GRID_SIZE)
			{
				prev = &battle.grid[x][y];
				
				for (cell = battle.grid[x][y].next ; cell != NULL ; cell = cell->next)
				{
					if (cell->entity == e)
					{
						prev->next = cell->next;
						free(cell);
						cell = prev;
					}
					
					prev = cell;
				}
			}
		}
	}
}

Entity **getAllEntsWithin(int x, int y, int w, int h, Entity *ignore)
{
	GridCell *cell;
	int x1, y1, x2, y2;
	
	memset(candidates, 0, sizeof(Entity*) * MAX_GRID_CANDIDATES);
	
	x1 = x / GRID_CELL_WIDTH;
	y1 = y / GRID_CELL_HEIGHT;
	x2 = (x + w) / GRID_CELL_WIDTH;
	y2 = (y + h) / GRID_CELL_HEIGHT;
	
	for (x = x1 ; x <= x2 ; x++)
	{
		for (y = y1 ; y <= y2 ; y++)
		{
			if (x >= 0 && y >= 0 && x < GRID_SIZE && y < GRID_SIZE)
			{
				for (cell = battle.grid[x][y].next ; cell != NULL ; cell = cell->next)
				{
					if (cell->entity != ignore)
					{
						addCandidate(cell->entity);
					}
				}
			}
		}
	}
	
	return candidates;
}

static void addCandidate(Entity *e)
{
	int i = 0;
	
	for (i = 0 ; i < MAX_GRID_CANDIDATES ; i++)
	{
		if (candidates[i] == NULL || candidates[i] == e)
		{
			candidates[i] = e;
			return;
		}
	}
	
	printf("Out of grid candidate space!");
	exit(1);
}

void destroyGrid(void)
{
	int x, y;
	GridCell *cell;
	
	for (x = 0 ; x < GRID_SIZE ; x++)
	{
		for (y = 0 ; y < GRID_SIZE ; y++)
		{
			while (battle.grid[x][y].next)
			{
				cell = battle.grid[x][y].next;
				battle.grid[x][y].next = cell->next;
				free(cell);
			}
		}
	}
}
