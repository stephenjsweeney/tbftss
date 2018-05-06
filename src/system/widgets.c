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

#include "widgets.h"

static void loadWidgets(void);
static void loadWidgetSet(char *filename);
static void handleMouse(void);
static void handleKeyboard(void);
static void createOptions(Widget *w, char *options);
static void changeSelectedValue(Widget *w, int dir);
static void createSelectButtons(Widget *w);
static void handleControlWidgets(void);

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

	optionsLeft = getTexture("gfx/widgets/optionsLeft.png");
	optionsRight = getTexture("gfx/widgets/optionsRight.png");

	loadWidgets();

	app.awaitingWidgetInput = drawingWidgets = 0;
}

void doWidgets(void)
{
	if (drawingWidgets)
	{
		handleMouse();

		handleKeyboard();
		
		if (app.awaitingWidgetInput)
		{
			handleControlWidgets();
		}
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
	mouseOver = 0;

	for (w = head.next; w != NULL ; w = w->next)
	{
		if ((app.modalDialog.type == MD_NONE || (app.modalDialog.type != MD_NONE && w->isModal)) && w->visible && strcmp(w->group, group) == 0)
		{
			if (!mouseOver && !app.awaitingWidgetInput)
			{
				mouseOver = (w->type != WT_SELECT && w->enabled && collision(w->rect.x, w->rect.y, w->rect.w, w->rect.h, app.mouse.x, app.mouse.y, 1, 1));

				if (mouseOver && selectedWidget != w)
				{
					if (w->type == WT_BUTTON || w->type == WT_CONTROL_CONFIG)
					{
						playSound(SND_GUI_CLICK);
					}

					selectedWidget = w;
				}
			}

			if (w->texture)
			{
				blit(w->texture , w->rect.x, w->rect.y, 0);
			}
			else
			{
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
					break;
					
				case WT_CONTROL_CONFIG:
					SDL_RenderDrawRect(app.renderer, &w->rect);
					
					if (!app.awaitingWidgetInput || (app.awaitingWidgetInput && w != selectedWidget))
					{
						if (strlen(w->options[0]) && strlen(w->options[1]))
						{
							drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, "%s or %s", w->options[0], w->options[1]);
						}
						else if (strlen(w->options[0]))
						{
							drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, "%s", w->options[0]);
						}
						else if (strlen(w->options[1]))
						{
							drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, "%s", w->options[1]);
						}
						else
						{
							drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, "");
						}
					}
					else
					{
						drawText(w->rect.x + (w->rect.w / 2), w->rect.y + 2, 20, TA_CENTER, colors.white, "...");
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

static void changeSelectedValue(Widget *w, int dir)
{
	int oldOption = w->currentOption;

	w->currentOption += dir;

	w->currentOption = MIN(MAX(0, w->currentOption), w->numOptions - 1);

	w->onChange(w->options[w->currentOption]);

	if (oldOption != w->currentOption)
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

static void handleMouse(void)
{
	Widget *old;

	if (selectedWidget && collision(selectedWidget->rect.x, selectedWidget->rect.y, selectedWidget->rect.w, selectedWidget->rect.h, app.mouse.x, app.mouse.y, 1, 1))
	{
		if (app.mouse.button[SDL_BUTTON_LEFT])
		{
			switch (selectedWidget->type)
			{
				case WT_BUTTON:
				case WT_IMG_BUTTON:
					if (selectedWidget->action)
					{
						playSound(SND_GUI_SELECT);
						old = selectedWidget;
						selectedWidget->action();
						if (old == selectedWidget)
						{
							selectedWidget = NULL;
						}
						app.mouse.button[SDL_BUTTON_LEFT] = 0;
					}
					break;

				case WT_SELECT_BUTTON:
					changeSelectedValue(selectedWidget->parent, selectedWidget->value);
					app.mouse.button[SDL_BUTTON_LEFT] = 0;
					break;
					
				case WT_CONTROL_CONFIG:
					if (!app.awaitingWidgetInput)
					{
						app.awaitingWidgetInput = 1;
						app.lastKeyPressed = app.lastButtonPressed = -1;
						playSound(SND_GUI_SELECT);
					}
					app.mouse.button[SDL_BUTTON_LEFT] = 0;
					break;
			}
		}
	}
}

static void handleKeyboard(void)
{
	Widget *old;
	
	if (selectedWidget != NULL)
	{
		if (selectedWidget->type == WT_BUTTON)
		{
			if (app.keyboard[SDL_SCANCODE_SPACE] ||app.keyboard[SDL_SCANCODE_RETURN])
			{
				playSound(SND_GUI_SELECT);
				old = selectedWidget;
				selectedWidget->action();

				if (old == selectedWidget)
				{
					selectedWidget = NULL;
				}

				app.keyboard[SDL_SCANCODE_SPACE] = app.keyboard[SDL_SCANCODE_RETURN] = 0;
			}
		}
	}
}

static void handleControlWidgets(void)
{
	if (app.lastKeyPressed == SDL_SCANCODE_BACKSPACE)
	{
		clearControlConfig(selectedWidget->name);
		
		app.awaitingWidgetInput = 0;
	}
	else if (app.lastKeyPressed == SDL_SCANCODE_ESCAPE)
	{
		playSound(SND_GUI_CLOSE);
		app.awaitingWidgetInput = 0;
	}
	else
	{
		if (app.lastKeyPressed != -1)
		{
			updateControlKey(selectedWidget->name);
			
			app.awaitingWidgetInput = 0;
		}
		
		if (app.lastButtonPressed != -1)
		{
			updateControlButton(selectedWidget->name);
			
			app.awaitingWidgetInput = 0;
		}
	}
	
	if (!app.awaitingWidgetInput)
	{
		clearInput();
	}
	
	app.lastKeyPressed = app.lastButtonPressed = -1;
}

static void loadWidgets()
{
	char **filenames;
	char path[MAX_FILENAME_LENGTH];
	int count, i;

	filenames = getFileList("data/widgets", &count);

	for (i = 0 ; i < count ; i++)
	{
		sprintf(path, "data/widgets/%s", filenames[i]);

		loadWidgetSet(path);

		free(filenames[i]);
	}

	free(filenames);
}

static void loadWidgetSet(char *filename)
{
	cJSON *root, *node;
	char *text;
	Widget *w;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	text = readFile(filename);
	root = cJSON_Parse(text);
	
	if (root)
	{
		for (node = root->child ; node != NULL ; node = node->next)
		{
			w = malloc(sizeof(Widget));
			memset(w, 0, sizeof(Widget));

			w->type = lookup(cJSON_GetObjectItem(node, "type")->valuestring);
			STRNCPY(w->name, cJSON_GetObjectItem(node, "name")->valuestring, MAX_NAME_LENGTH);
			STRNCPY(w->group, cJSON_GetObjectItem(node, "group")->valuestring, MAX_NAME_LENGTH);
			w->rect.x = cJSON_GetObjectItem(node, "x")->valueint;
			w->rect.y = cJSON_GetObjectItem(node, "y")->valueint;
			w->enabled = 1;
			w->visible = 1;

			if (w->rect.x == -1)
			{
				w->rect.x = SCREEN_WIDTH / 2;
			}

			switch (w->type)
			{
				case WT_BUTTON:
					STRNCPY(w->text, _(cJSON_GetObjectItem(node, "text")->valuestring), MAX_NAME_LENGTH);
					w->rect.w = cJSON_GetObjectItem(node, "w")->valueint;
					w->rect.h = cJSON_GetObjectItem(node, "h")->valueint;
					w->rect.x -= w->rect.w / 2;
					w->rect.y -= (w->rect.h / 2) + 8;
					break;

				case WT_IMG_BUTTON:
					w->texture = getTexture(cJSON_GetObjectItem(node, "texture")->valuestring);
					SDL_QueryTexture(w->texture, NULL, NULL, &w->rect.w, &w->rect.h);
					break;

				case WT_SELECT:
					STRNCPY(w->text, _(cJSON_GetObjectItem(node, "text")->valuestring), MAX_NAME_LENGTH);
					w->rect.w = cJSON_GetObjectItem(node, "w")->valueint;
					w->rect.h = cJSON_GetObjectItem(node, "h")->valueint;
					w->rect.x -= w->rect.w / 2;
					w->rect.y -= (w->rect.h / 2) + 8;
					createSelectButtons(w);
					createOptions(w, cJSON_GetObjectItem(node, "options")->valuestring);
					break;
					
				case WT_CONTROL_CONFIG:
					w->rect.w = cJSON_GetObjectItem(node, "w")->valueint;
					w->rect.h = cJSON_GetObjectItem(node, "h")->valueint;
					break;
					
				default:
					printf("Widget type %d not handled\n", w->type);
					exit(1);
					break;
			}

			tail->next = w;
			tail = w;
		}

		cJSON_Delete(root);
	}
	else
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, "Failed to load '%s'", filename);
	}
	
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

static void createSelectButtons(Widget *w)
{
	int i;
	Widget *btn;

	for (i = 0 ; i < 2 ; i++)
	{
		btn = malloc(sizeof(Widget));
		memcpy(btn, w, sizeof(Widget));
		strcpy(btn->name, "");

		btn->type = WT_SELECT_BUTTON;
		btn->parent = w;

		if (i == 0)
		{
			btn->value = -1;
			btn->rect.x -= 32;
			btn->rect.y += 4;
			btn->texture = optionsLeft;
		}
		else
		{
			btn->value = 1;
			btn->rect.x += btn->rect.w + 8;
			btn->rect.y += 4;
			btn->texture = optionsRight;
		}

		tail->next = btn;
		tail = btn;
	}
}

void autoSizeWidgetButtons(char *group, int recenter)
{
	int width, height, maxWidth;
	Widget *w;
	
	maxWidth = 0;
	
	for (w = head.next; w != NULL ; w = w->next)
	{
		if (strcmp(w->group, group) == 0 && w->type == WT_BUTTON)
		{
			textSize(w->text, 20, &width, &height);
			
			maxWidth = MAX(MAX(w->rect.w, width), maxWidth);
		}
	}
	
	for (w = head.next; w != NULL ; w = w->next)
	{
		if (strcmp(w->group, group) == 0 && w->type == WT_BUTTON)
		{
			w->rect.w = maxWidth + 20;
			
			if (recenter)
			{
				w->rect.x = (SCREEN_WIDTH / 2) - (w->rect.w / 2);
			}
		}
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
		
		free(w->options);

		next = w->next;
		free(w);
		w = next;
	}

	head.next = NULL;
}
