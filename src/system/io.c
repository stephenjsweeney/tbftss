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

#include "io.h"

int fileExists(char *filename)
{
	struct stat buffer;
	
	return (stat(filename, &buffer) == 0);
}

char *readFile(char *filename)
{
	char *buffer = 0;
	long length;
	FILE *file = fopen(filename, "rb");

	if (file)
	{
		fseek(file, 0, SEEK_END);
		length = ftell(file);
		fseek(file, 0, SEEK_SET);

		buffer = malloc(length);
		memset(buffer, 0, length);
		fread(buffer, 1, length, file);

		fclose(file);
	}

	return buffer;
}

int writeFile(char *filename, char *data)
{
	FILE *file = fopen(filename, "wb");
	
	if (file)
	{
		fprintf(file, "%s\n", data);
		fclose(file);
		return 1;
	}
	
	return 0;
}

char *getSaveFilePath(char *filename)
{
	static char path[MAX_FILENAME_LENGTH];
	
	sprintf(path, "%s/%s", app.saveDir, filename);
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "getSaveFilePath = '%s'", path);
	
	return path;
}
