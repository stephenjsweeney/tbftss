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

int           stringComparator(const void *a, const void *b);
unsigned long hashcode(const char *str);
void		 *resize(void *array, int oldSize, int newSize);
int           getJSONValue(struct cJSON *node, char *name, int defValue);
char		 *getJSONValueStr(struct cJSON *node, char *name, char *defValue);
char		 *timeToDate(long millis);
char		 *timeToString(long millis, int showHours);
char		**toTypeArray(char *types, int *numTypes);
int           getDistance(int x1, int y1, int x2, int y2);
float         getAngle(int x1, int y1, int x2, int y2);
int           collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
int           getPercent(float current, float total);
float         mod(float n, float x);
