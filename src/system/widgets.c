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

#include "widgets.h"

static void loadWidgets(char *filename);
static void loadWidgetSet(char *filename);
static void handleKeyboard(void);
static void handleMouse(void);
static void createOptions(Widget *w, char *options);

static Widget head;
static Widget *tail;
static Widget *selectedWidget;
static SDL_Texture *optionsLeft;
static SDL_Texture *optionsRight;
static int drawingWidgets;

void initWidgets(void)
{
	memset(&head, 0, sizeof(Widget));
	
	tail = &head;
	
	selectedWidget = NULL;
	
	loadWidgets("data/widgets/list.json");
	
	drawingWidgets = 0;
	
	optionsLeft = getTexture("gfx/widgets/optionsLeft.png");
	optionsRight = getTexture("gfx/widgets/optionsRight.png");
}

void doWidgets(void)
{
	if (drawingWidgets)
	{
		handleKeyboard();
		
		handleMouse();
	}
	
	drawingWidgets = 0;
}

Widget *getWidget(const char *name, const char *group)
{
	Widget *w;

	for (w = head.next; w != NULL ; w = w->next)
	{
		if (strcmp(w->name, name) == 0 && strcmp(w->group, group) == 0)
		{
			return w;
		}
	}

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "No such widget ('%s', '%s')", name, group);

	return NULL;
}

void selectWidget(const char *name, const char *group)
{
	selectedWidget = getWidget(name, group);
}

void drawWidgets(const char *group)
{
	int mouseOver;
	Widget *w;
	
	drawingWidgets = 1;
	
	for (w = head.next; w != NULL ; w = w->next)
	{
		if (w->visible && strcmp(w->group, group) == 0)
		{
			mouseOver = (w->enabled && collision(w->rect.x, w->rect.y, w->rect.w, w->rect.h, app.mouse.x, app.mouse.y, 1, 1));
			
			if (mouseOver && selectedWidget != w)
			{
				playSound(SND_GUI_CLICK);
				selectedWidget = w;
			}
			
			SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(app.renderer, &w->rect);
			
			if (w == selectedWidget)
			{
				SDL_SetRenderDrawColor(app.renderer, 64, 128, 200, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(app.renderer, &w->rect);
				SDL_SetRenderDrawColor(app.renderer, 128, 192, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawRect(app.renderer, &w->rect);
			}
			else
			{
				SDL_SetRenderDrawColor(app.renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawRect(app.renderer, &w->rect);
			}
			
			switch (w->type)
			{
				case WT_BUTTON:
					SDL_RenderDrawRect(app.renderer, &w->rect);
					drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, w->text);
					break;
					
				case WT_SELECT:
					drawText(w->rect.x + 10, w->rect.y + 2, 20, TA_LEFT, colors.white, w->text);
					drawText(w->rect.x + w->rect.w - 10, w->rect.y + 2, 20, TA_RIGHT, colors.white, w->options[w->currentOption]);
					
					if (w->currentOption != 0)
					{
						blit(optionsLeft, w->rect.x - 24, w->rect.y + 16, 1);
					}
					
					if (w->currentOption != w->numOptions - 1)
					{
						blit(optionsRight, w->rect.x + w->rect.w + 24, w->rect.y + 16, 1);
					}
					
					break;
			}

			if (!w->enabled)
			{
				SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 192);
				SDL_RenderFillRect(app.renderer, &w->rect);
				SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_NONE);
			}
		}
	}
}

void drawConfirmMessage(char *message)
{
	drawWidgets("okCancel");
}

static void gotoWidget(int dx, int dy)
{
	Widget *w, *closest;
	int distance;
	int curDistance = -1;
	
	closest = selectedWidget;
	
	for (w = head.next; w != NULL ; w = w->next)
	{
		if (w == selectedWidget || 
			!w->enabled ||
			!w->visible ||
			strcmp(w->group, selectedWidget->group) != 0 ||
			(dx == -1 && w->rect.x > selectedWidget->rect.x) ||
			(dx == 1 && w->rect.x < selectedWidget->rect.x) ||
			(dx != 0 && w->rect.x == selectedWidget->rect.x) ||
			(dy == -1 && w->rect.y > selectedWidget->rect.y) ||
			(dy == 1 && w->rect.y < selectedWidget->rect.y) ||
			(dy != 0 && w->rect.y == selectedWidget->rect.y)
		)
		{
			continue;
		}
		
		distance = getDistance(w->rect.x, w->rect.y, selectedWidget->rect.x, selectedWidget->rect.y);
		
		if (curDistance == -1 || distance < curDistance)
		{
			curDistance = distance;
			closest = w;
		}
	}
	
	if (selectedWidget != closest)
	{
		playSound(SND_GUI_CLICK);
			
		selectedWidget = closest;
	}
}

static void changeSelectValue(int dir)
{
	int oldOption = selectedWidget->currentOption;
	
	selectedWidget->currentOption += dir;
	
	selectedWidget->currentOption = MIN(MAX(0, selectedWidget->currentOption), selectedWidget->numOptions - 1);
	
	selectedWidget->onChange(selectedWidget->options[selectedWidget->currentOption]);
	
	if (oldOption != selectedWidget->currentOption)
	{
		playSound(SND_GUI_CLICK);
	}
}

void setWidgetOption(const char *name, const char *group, const char *value)
{
	int i;
	Widget *w = getWidget(name, group);
	
	if (w)
	{
		for (i = 0 ; i < w->numOptions ; i++)
		{
			if (strcmp(w->options[i], value) == 0)
			{
				w->currentOption = i;
				return;
			}
		}
	}
}

static void handleKeyboard(void)
{
	if (app.keyboard[SDL_SCANCODE_UP])
	{
		gotoWidget(0, -1);
	}
	
	if (app.keyboard[SDL_SCANCODE_DOWN])
	{
		gotoWidget(0, 1);
	}
	
	if (app.keyboard[SDL_SCANCODE_LEFT])
	{
		if (selectedWidget->type != WT_SELECT)
		{
			gotoWidget(-1, 0);
		}
		else
		{
			changeSelectValue(-1);
		}
	}
	
	if (app.keyboard[SDL_SCANCODE_RIGHT])
	{
		if (selectedWidget->type != WT_SELECT)
		{
			gotoWidget(1, 0);
		}
		else
		{
			changeSelectValue(1);
		}
	}
	
	if (app.keyboard[SDL_SCANCODE_RETURN] && selectedWidget->action)
	{
		playSound(SND_GUI_SELECT);
		selectedWidget->action();
	}
	
	memset(app.keyboard, 0, sizeof(int) * MAX_KEYBOARD_KEYS);
}

static void handleMouse(void)
{
	if (selectedWidget && selectedWidget->action && app.mouse.button[SDL_BUTTON_LEFT])
	{
		if (collision(selectedWidget->rect.x, selectedWidget->rect.y, selectedWidget->rect.w, selectedWidget->rect.h, app.mouse.x, app.mouse.y, 1, 1))
		{
			playSound(SND_GUI_SELECT);
			selectedWidget->action();
		}
	}
}

static void loadWidgets(char *filename)
{
	cJSON *root, *node;
	char *text;
	
	text = readFile(filename);
	root = cJSON_Parse(text);
	
	for (node = root->child ; node != NULL ; node = node->next)
	{
		loadWidgetSet(node->valuestring);
	}
	
	cJSON_Delete(root);
	free(text);
}

static void loadWidgetSet(char *filename)
{
	cJSON *root, *node;
	char *text;
	Widget *w;
	
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);
	root = cJSON_Parse(text);

	for (node = root->child ; node != NULL ; node = node->next)
	{
		w = malloc(sizeof(Widget));
		memset(w, 0, sizeof(Widget));
		
		w->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
		STRNCPY(w->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
		STRNCPY(w->group, cJSON_GetObjectItem(node, "group")->valuestring, MAX_NAME_LENGTH);
		w->rect.x = cJSON_GetObjectItem(node, "x")->valueint;
		w->rect.y = cJSON_GetObjectItem(node, "y")->valueint;
		w->rect.w = cJSON_GetObjectItem(node, "w")->valueint;
		w->rect.h = cJSON_GetObjectItem(node, "h")->valueint;
		w->enabled = 1;
		w->visible = 1;
		
		if (w->rect.x == -1)
		{
			w->rect.x = SCREEN_WIDTH / 2;
		}
		
		switch (w->type)
		{
			case WT_BUTTON:
				STRNCPY(w->text, cJSON_GetObjectItem(node, "text")->valuestring, MAX_NAME_LENGTH);
				w->rect.x -= w->rect.w / 2;
				w->rect.y -= (w->rect.h / 2) + 8;
				break;
				
			case WT_SELECT:
				STRNCPY(w->text, cJSON_GetObjectItem(node, "text")->valuestring, MAX_NAME_LENGTH);
				w->rect.x -= w->rect.w / 2;
				w->rect.y -= (w->rect.h / 2) + 8;
				createOptions(w, cJSON_GetObjectItem(node, "options")->valuestring);
				break;
		}
	
		tail->next = w;
		tail = w;
	}

	cJSON_Delete(root);
	free(text);
}

static void createOptions(Widget *w, char *options)
{
	int i;
	char *option;
	
	w->numOptions = 1;
	
	for (i = 0 ; i < strlen(options) ; i++)
	{
		if (options[i] == ';')
		{
			w->numOptions++;
		}
	}
	
	w->options = malloc(w->numOptions * sizeof(char*));
	
	i = 0;
	option = strtok(options, ";");
	while (option)
	{
		w->options[i] = malloc(strlen(option) + 1);
		strcpy(w->options[i], option);
		
		option = strtok(NULL, ";");
		
		i++;
	}
}

void destroyWidgets(void)
{
	int i;
	Widget *w = head.next;
	Widget *next;

	while (w)
	{
		for (i = 0 ; i < w->numOptions ; i++)
		{
			free(w->options[i]);
		}
		
		next = w->next;
		free(w);
		w = next;
	}

	head.next = NULL;
}
