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

#include <dirent.h>
#include <sys/stat.h>

#include "../common.h"

#include "../system/util.h"
#include "io.h"

extern App app;

int fileExists(char *filename)
{
	struct stat buffer;

	return (stat(filename, &buffer) == 0);
}

char *getFileLocation(char *filename)
{
	static char path[MAX_FILENAME_LENGTH];
	memset(path, '\0', MAX_FILENAME_LENGTH);

	if (fileExists(filename))
	{
		return filename;
	}

	sprintf(path, DATA_DIR "/%s", filename);

	return path;
}

char *readFile(char *filename)
{
	char *buffer = NULL;
	long  length;
	FILE *file;

	file = fopen(getFileLocation(filename), "rb");

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
	static char path[MAX_PATH_LENGTH];
	memset(path, '\0', MAX_PATH_LENGTH);

	sprintf(path, "%s/%s", app.saveDir, filename);

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "getSaveFilePath = '%s'", path);

	return path;
}

char **getFileList(char *dir, int *count)
{
	DIR		   *d;
	int			   i;
	struct dirent *ent;
	char		 **filenames;

	i = 0;
	filenames = NULL;

	if ((d = opendir(getFileLocation(dir))) != NULL)
	{
		while ((ent = readdir(d)) != NULL)
		{
			if (ent->d_name[0] != '.')
			{
				i++;
			}
		}

		if (i > 0)
		{
			filenames = malloc(sizeof(char *) * i);
			memset(filenames, 0, sizeof(char *) * i);

			rewinddir(d);

			i = 0;

			while ((ent = readdir(d)) != NULL)
			{
				if (ent->d_name[0] != '.')
				{
					filenames[i] = malloc(sizeof(char) * MAX_FILENAME_LENGTH);

					STRNCPY(filenames[i], ent->d_name, MAX_FILENAME_LENGTH);

					i++;
				}
			}
		}

		closedir(d);
	}

	*count = i;

	if (filenames)
	{
		qsort(filenames, i, sizeof(char *), stringComparator);
	}

	return filenames;
}
