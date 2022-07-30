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

void clearInput(void);
void doInput(void);
void drawMouse(void);
void setMouseCursor(int isDrag);
void doMouseMotion(SDL_MouseMotionEvent *event);
void doMouseWheel(SDL_MouseWheelEvent *event);
void doMouseUp(SDL_MouseButtonEvent *event);
void doMouseDown(SDL_MouseButtonEvent *event);
void doKeyUp(SDL_KeyboardEvent *event);
void doKeyDown(SDL_KeyboardEvent *event);
void initInput(void);
