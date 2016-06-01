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

#include "script.h"

static void executeNextLine(ScriptRunner *runner);
void destroyScript(void);

static cJSON *scriptJSON, *rootJSON;
static ScriptRunner head;
static ScriptRunner *tail;
static int runScript;

void initScript(cJSON *root)
{
	cJSON *function;
	
	memset(&head, 0, sizeof(ScriptRunner));
	tail = &head;
	
	rootJSON = root;
	
	runScript = 0;

	scriptJSON = cJSON_GetObjectItem(root, "script");
	
	if (scriptJSON)
	{
		function = scriptJSON->child;

		while (function)
		{
			SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Found script function: '%s'", cJSON_GetObjectItem(function, "function")->valuestring);
			
			function = function->next;
		}
		
		runScript = 1;
	}
}

void doScript(void)
{
	ScriptRunner *runner, *prev;

	if (runScript)
	{
		prev = &head;

		for (runner = head.next ; runner != NULL ; runner = runner->next)
		{
			runner->delay = MAX(0, runner->delay - 1);

			if (runner->waitForMessageBox)
			{
				runner->waitForMessageBox = showingMessageBoxes();
			}

			if (!runner->delay && !runner->waitForMessageBox)
			{
				executeNextLine(runner);

				if (!runner->line)
				{
					if (runner == tail)
					{
						tail = prev;
					}

					prev->next = runner->next;
					free(runner);
					runner = prev;
				}
			}

			prev = runner;
		}
	}
}

void runScriptFunction(const char *format, ...)
{
	ScriptRunner *scriptRunner;
	cJSON *function;
	char *functionName;
	char funcNameBuffer[MAX_NAME_LENGTH];
	va_list args;

	if (scriptJSON && runScript)
	{
		memset(&funcNameBuffer, '\0', sizeof(funcNameBuffer));

		va_start(args, format);
		vsprintf(funcNameBuffer, format, args);
		va_end(args);

		function = scriptJSON->child;

		while (function)
		{
			functionName = cJSON_GetObjectItem(function, "function")->valuestring;

			if (strcmp(functionName, funcNameBuffer) == 0)
			{
				scriptRunner = malloc(sizeof(ScriptRunner));
				memset(scriptRunner, 0, sizeof(ScriptRunner));

				scriptRunner->line = cJSON_GetObjectItem(function, "lines")->child;

				tail->next = scriptRunner;
				tail = scriptRunner;

				SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Running script '%s'", funcNameBuffer);
				
				return;
			}

			function = function->next;
		}
	}
}

void runScriptTimeFunctions(void)
{
	ScriptRunner *scriptRunner;
	cJSON *function;
	char *functionName;
	char funcNameBuffer[MAX_NAME_LENGTH];
	
	if (scriptJSON && runScript)
	{
		function = scriptJSON->child;
		
		sprintf(funcNameBuffer, "TIME %d", battle.stats[STAT_TIME] / 60);
		
		while (function)
		{
			functionName = cJSON_GetObjectItem(function, "function")->valuestring;
			
			if (strcmp(functionName, funcNameBuffer) == 0)
			{
				scriptRunner = malloc(sizeof(ScriptRunner));
				memset(scriptRunner, 0, sizeof(ScriptRunner));

				scriptRunner->line = cJSON_GetObjectItem(function, "lines")->child;

				tail->next = scriptRunner;
				tail = scriptRunner;

				SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Running script '%s'", funcNameBuffer);
			}
			
			function = function->next;
		}
	}
}

static void executeNextLine(ScriptRunner *runner)
{
	char *line;
	char command[32];
	char strParam[3][256];
	int intParam[2];

	line = runner->line->valuestring;

	sscanf(line, "%s", command);

	if (strcmp(command, "ACTIVATE_ENTITIES") == 0)
	{
		sscanf(line, "%*s %[^\n]", strParam[0]);
		activateEntities(strParam[0]);
	}
	else if (strcmp(command, "ACTIVATE_ENTITY_GROUPS") == 0)
	{
		sscanf(line, "%*s %[^\n]", strParam[0]);
		activateEntityGroups(strParam[0]);
	}
	else if (strcmp(command, "ACTIVATE_OBJECTIVES") == 0)
	{
		sscanf(line, "%*s %[^\n]", strParam[0]);
		activateObjectives(strParam[0]);
	}
	else if (strcmp(command, "ACTIVATE_LOCATION") == 0)
	{
		sscanf(line, "%*s %[^\n]", strParam[0]);
		activateLocations(strParam[0]);
	}
	else if (strcmp(command, "ACTIVATE_JUMPGATE") == 0)
	{
		sscanf(line, "%*s %d", &intParam[0]);
		activateJumpgate(intParam[0]);
	}
	else if (strcmp(command, "ACTIVATE_NEXT_WAYPOINT") == 0)
	{
		activateNextWaypoint();
	}
	else if (strcmp(command, "ACTIVATE_SPAWNER") == 0)
	{
		sscanf(line, "%*s %d %[^\n]", &intParam[0], strParam[0]);
		activateSpawner(strParam[0], intParam[0]);
	}
	else if (strcmp(command, "MSG_BOX") == 0)
	{
		sscanf(line, "%*s %255[^;]%*c%255[^\n]", strParam[0], strParam[1]);
		addMessageBox(strParam[0], _(strParam[1]), MB_NORMAL);
	}
	else if (strcmp(command, "IMPORTANT_MSG_BOX") == 0)
	{
		sscanf(line, "%*s %255[^;]%*c%255[^\n]", strParam[0], strParam[1]);
		addMessageBox(strParam[0], _(strParam[1]), MB_IMPORTANT);
	}
	else if (strcmp(command, "PANDORAN_MSG_BOX") == 0)
	{
		sscanf(line, "%*s %255[^;]%*c%255[^\n]", strParam[0], strParam[1]);
		addMessageBox(strParam[0], _(strParam[1]), MB_PANDORAN);
	}
	else if (strcmp(command, "WAIT") == 0)
	{
		sscanf(line, "%*s %d", &intParam[0]);
		runner->delay = intParam[0] * FPS;
	}
	else if (strcmp(command, "WAIT_MSG_BOX") == 0)
	{
		runner->waitForMessageBox = 1;
	}
	else if (strcmp(command, "COMPLETE_MISSION") == 0)
	{
		addHudMessage(colors.green, _("Mission Complete!"));
		completeAllObjectives();
		completeMission();
	}
	else if (strcmp(command, "END_MISSION") == 0)
	{
		battle.unwinnable = 1;
		completeMission();
		battle.missionFinishedTimer = -FPS * 999;
	}
	else if (strcmp(command, "FAIL_MISSION") == 0)
	{
		addHudMessage(colors.red, _("Mission Failed!"));
		failMission();
	}
	else if (strcmp(command, "FAIL_CHALLENGE") == 0)
	{
		addHudMessage(colors.red, _("Challenge Failed!"));
		failMission();
	}
	else if (strcmp(command, "END_CHALLENGE") == 0)
	{
		battle.scriptedEnd = 1;
	}
	else if (strcmp(command, "RETREAT_ALLIES") == 0)
	{
		battle.isEpic = 0;
		retreatAllies();
	}
	else if (strcmp(command, "RETREAT_ENEMIES") == 0)
	{
		battle.isEpic = 0;
		retreatEnemies();
	}
	else if (strcmp(command, "CREATE_CRISTABEL_LOCATION") == 0)
	{
		createChristabelLocation();
	}
	else if (strcmp(command, "KILL_ENTITY") == 0)
	{
		sscanf(line, "%*s %[^\n]", strParam[0]);
		killEntity(strParam[0]);
	}
	else if (strcmp(command, "UPDATE_ENTITY_SIDE") == 0)
	{
		sscanf(line, "%*s %s %[^\n]", strParam[0], strParam[1]);
		updateEntitySide(strParam[0], strParam[1]);
	}
	else if (strcmp(command, "ACTIVATE_TRESPASSER_SPAWNER") == 0)
	{
		activateTrespasserSpawner();
	}
	else if (strcmp(command, "STOP_SCRIPT") == 0)
	{
		runScript = 0;
	}
	else if (strcmp(command, "DESTROY_TORELLI") == 0)
	{
		battle.destroyTorelli = 1;
	}
	else if (strcmp(command, "END_CAMPAIGN") == 0)
	{
		battle.campaignFinished = 1;
	}
	else
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "ERROR: Unrecognised script command '%s'\n", command);
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "ERROR: Offending line: '%s'\n", line);
	}

	runner->line = runner->line->next;
}

void cancelScript(void)
{
	ScriptRunner *runner;
	
	while (head.next)
	{
		runner = head.next;
		head.next = runner->next;
		free(runner);
	}
	
	tail = &head;
}

void destroyScript(void)
{
	ScriptRunner *scriptRunner;
	
	if (rootJSON)
	{
		cJSON_Delete(rootJSON);
		
		rootJSON = NULL;
	}

	while (head.next)
	{
		scriptRunner = head.next;
		head.next = scriptRunner->next;
		free(scriptRunner);
	}

	tail = &head;
	
	scriptJSON = NULL;
}
