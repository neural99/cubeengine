/*
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 3 of the License, or 
 *  (at your option) any later version. 

 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 

 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <SDL/SDL.h>
#include "startup.h"

typedef struct event_handler_s {
	Uint8 type_filter;
	int (*callback)(SDL_Event *event);
} event_handler_t;

typedef struct keypress_handler_s {
	int repeat_interval;
	SDLKey sym;
	SDL_TimerID timer;
	SDL_Event *repeated_event;
	int (*callback)(SDL_Event *event);
} keypress_handler_t;

void event_init(void);

void event_add_keypress_handler(keypress_handler_t *k);
void event_remove_keypress_handler(keypress_handler_t *k);
void event_add_event_handler(event_handler_t *h);
void event_remove_event_handler(event_handler_t *h);

void event_dispatch(SDL_Event *event);

void event_cleanup(void);

#endif
