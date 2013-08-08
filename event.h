#ifndef __EVENT_H__
#define __EVENT_H__

#include <SDL/sdl.h>
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
