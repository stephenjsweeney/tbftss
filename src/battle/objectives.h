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

void addEpicKillsObjective(void);
void addEpicLivesObjective(void);
void loadObjectives(struct cJSON *node);
void activateObjectives(char *objectives);
void failIncompleteObjectives(void);
void completeConditions(void);
void completeAllObjectives(void);
void updateCondition(char *name, int type);
void adjustObjectiveTargetValue(char *name, int type, int amount);
void updateObjective(char *name, int type);
void doObjectives(void);
