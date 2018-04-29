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

#include "util.h"

float mod(float n, float x)
{
	return fmod(fmod(n, x) + x, x);
}

int getPercent(float current, float total)
{
	return total != 0 ? (current / total) * 100 : 0;
}

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	return (MAX(x1, x2) < MIN(x1 + w1, x2 + w2)) && (MAX(y1, y2) < MIN(y1 + h1, y2 + h2));
}

float getAngle(int x1, int y1, int x2, int y2)
{
	float angle = -90 + atan2(y1 - y2, x1 - x2) * (180 / PI);
	return angle >= 0 ? angle : 360 + angle;
}

int getDistance(int x1, int y1, int x2, int y2)
{
	int x, y;

	x = x2 - x1;
	y = y2 - y1;

	return sqrt(x * x + y *y);
}

char **toTypeArray(char *types, int *numTypes)
{
	int i;
	char **typeArray, *type;

	*numTypes = 1;

	for (i = 0 ; i < strlen(types) ; i++)
	{
		if (types[i] == ';')
		{
			*numTypes = *numTypes + 1;
		}
	}

	typeArray = malloc(*numTypes * sizeof(char*));

	i = 0;
	type = strtok(types, ";");
	while (type)
	{
		typeArray[i] = malloc(strlen(type) + 1);
		strcpy(typeArray[i], type);

		type = strtok(NULL, ";");

		i++;
	}

	return typeArray;
}

char *timeToString(long millis, int showHours)
{
	static char TIME[MAX_NAME_LENGTH];

	int hours, minutes, seconds;

	seconds = millis / FPS;
	minutes = (seconds / FPS) % 60;
	hours = seconds / (FPS * FPS);
	seconds %= 60;

	if (showHours)
	{
		sprintf(TIME, "%dh %02dm %02ds", hours, minutes, seconds);
	}
	else
	{
		sprintf(TIME, "%dm %02ds", minutes, seconds);
	}

	return TIME;
}

char *timeToDate(long millis)
{
	static char DATE[MAX_NAME_LENGTH];
	
	struct tm *timeinfo;
	
	timeinfo = localtime(&millis);
	
	strftime(DATE, MAX_NAME_LENGTH, "%d %b %Y, %I:%M%p", timeinfo);
	
	return DATE;
}

char *getJSONValueStr(cJSON *node, char *name, char *defValue)
{
	cJSON *child;

	if (node)
	{
		child = cJSON_GetObjectItem(node, name);

		if (child)
		{
			return child->valuestring;
		}
	}

	return defValue;
}

int getJSONValue(cJSON *node, char *name, int defValue)
{
	cJSON *child;

	if (node)
	{
		child = cJSON_GetObjectItem(node, name);

		if (child)
		{
			return child->valueint;
		}
	}

	return defValue;
}

void *resize(void *array, int oldSize, int newSize)
{
	void **newArray;
	int copySize;
	
	copySize = newSize > oldSize ? oldSize : newSize;
	
	newArray = malloc(newSize);
	memset(newArray, 0, newSize);
	memcpy(newArray, array, copySize);
	free(array);
	
	return newArray;
}
