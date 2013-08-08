#include <SDL/sdl.h>
#include "console.h"
#include "event.h"
#include "util.h"
#include "startup.h"

static int console_isactive = 0;
static char console_line_buff[2048];

static event_handler_t *keydown_handler;

static void
add_char(char ch){
	char *b = console_line_buff;
	while(*b != 0) b++;
	*b++ = ch;
	*b++ = 0;
}

static void
new_line(void){
	char *n = "> ";
	strcpy(console_line_buff, n);
}

static int 
keydown_callback(SDL_Event *event){
	if(console_isactive){
		if(event->key.keysym.sym == SDLK_RETURN){
			LOG_DEBUG("Console deactived");
			console_isactive = 0;
			return 1;
		}else{
			char ch = event->key.keysym.unicode & 0x7F;
			add_char(ch);
			return 1;
		}
	}else{
		if(event->key.keysym.sym == SDLK_RETURN){
			LOG_DEBUG("Console actived");
			console_isactive = 1;
			return 1;
		}
	}
	return 0;
}

void
console_init(void){
	keydown_handler = malloc(sizeof(event_handler_t));
	keydown_handler->type_filter = SDL_KEYDOWN;
	keydown_handler->callback = keydown_callback;
	event_add_event_handler(keydown_handler);
}
STARTUP_PROC(console, 6, console_init)

void
console_draw(void){
	if(console_isactive){
	}
}

int
console_active(void){
	return console_isactive;
}

