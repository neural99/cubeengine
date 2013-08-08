#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "event.h"
#include "util.h"

static linked_list_t *event_handler_list = NULL;
static linked_list_t *keypress_handler_list = NULL;

static event_handler_t *keypress_keydown_handler; 
static event_handler_t *keypress_keyup_handler; 

static keypress_handler_t* keypress_handler(SDL_Event *e);
static void add_keypress_timer(keypress_handler_t *h, SDL_Event *e);
static void remove_keypress_timer(keypress_handler_t *h);
static int handle_keypress_handlers(SDL_Event *e);

void
event_init(void){
	event_handler_list = util_list_create();
	keypress_handler_list = util_list_create();

	keypress_keydown_handler = malloc(sizeof(event_handler_t));
	keypress_keydown_handler->type_filter = SDL_KEYDOWN;
	keypress_keydown_handler->callback = handle_keypress_handlers;
	event_add_event_handler(keypress_keydown_handler);

	keypress_keyup_handler = malloc(sizeof(event_handler_t));
	keypress_keyup_handler->type_filter = SDL_KEYUP;
	keypress_keyup_handler->callback = handle_keypress_handlers;
	event_add_event_handler(keypress_keyup_handler);
} 
STARTUP_PROC(event, 1, event_init)

void
event_cleanup(void){
	event_remove_event_handler(keypress_keydown_handler);
	free(keypress_keydown_handler);

	event_remove_event_handler(keypress_keyup_handler);
	free(keypress_keyup_handler);

	util_list_free(event_handler_list);
	util_list_free(keypress_handler_list);
}

void
event_add_keypress_handler(keypress_handler_t *h){
	if(keypress_handler_list != NULL)
		util_list_add(keypress_handler_list, h);
}

void
event_remove_keypress_handler(keypress_handler_t *h){
	if(keypress_handler_list != NULL)
		util_list_remove(keypress_handler_list, h);
}

void
event_add_event_handler(event_handler_t *h){
	if(event_handler_list != NULL)
		util_list_insert(event_handler_list, h);
}

void 
event_remove_event_handler(event_handler_t *h){
	if(event_handler_list != NULL)
		util_list_remove(event_handler_list, h);
}

void
event_dispatch(SDL_Event *ev){
	if(event_handler_list == NULL)
		FATAL_ERROR("event_handler_list is NULL in event_dispatch. Should not happen. Aborting.");

	linked_list_elm_t *e;

	e = event_handler_list->head;
	while(e != NULL){
		event_handler_t *eh = e->data;
		if(eh->type_filter == ev->type){
			int consumed = eh->callback(ev);	
			if(consumed)
				return;
		}

		e = e->next;
	}
}

static int
handle_keypress_handlers(SDL_Event *e){
	if(e->type == SDL_KEYDOWN){
		keypress_handler_t *h;
		h = keypress_handler(e);
		if(h != NULL){
			if(h->timer == 0) 
				add_keypress_timer(h, e);
			h->callback(e);
		}
	} else if (e->type == SDL_KEYUP){
		keypress_handler_t *h;
		h = keypress_handler(e);
		if(h != NULL && h->timer != 0)
			remove_keypress_timer(h);

	}
	return 0;
}	

static Uint32 
timer_callback(Uint32 interval, void *param){
	SDL_Event *ep = param;
	SDL_PushEvent(ep);

	return interval;
}

static keypress_handler_t*
keypress_handler(SDL_Event *e){
	linked_list_elm_t *elm;
	linked_list_elm_t *matched;

	matched = NULL;
	elm = keypress_handler_list->head;
	while(elm != NULL){
		keypress_handler_t *handler;
		handler = elm->data;
		if (handler->sym == e->key.keysym.sym){
			matched = elm;
			break;
		}
		elm = elm->next;
	}
	if(matched)
		return matched->data;
	else
		return NULL;
}

static void
add_keypress_timer(keypress_handler_t *h, SDL_Event *e){
	SDL_Event *e_cpy;
	e_cpy = malloc(sizeof(SDL_Event));
	memcpy(e_cpy, e, sizeof(SDL_Event));
	h->timer = SDL_AddTimer(h->repeat_interval, timer_callback, e_cpy);
	h->repeated_event = e_cpy;
}	

static void
remove_keypress_timer(keypress_handler_t *h){
	SDL_RemoveTimer(h->timer);
	h->timer = 0;
	free(h->repeated_event);
}


