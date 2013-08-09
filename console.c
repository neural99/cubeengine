#include <SDL/sdl.h>
#include "console.h"
#include "event.h"
#include "util.h"
#include "startup.h"
#include "hud.h"

static int console_isactive = 0;
static int draw_cursor = 0;
static char console_line_buff[2048];
static linked_list_t *console_text;
static float console_fadeout_alpha = 1.0;

static event_handler_t *keydown_handler;
static anim_task_t *cursor_task;
static anim_task_t *fadeout_task;

static int 
isempty(void){
	return console_line_buff[0] == 0;
}

static void
backspace(void){
	char *c = console_line_buff;
	while(*c != 0) c++;
	if(c != console_line_buff)
		*(c-1) = 0;
}

static void
add_char(char ch){
	printf("%c\n", ch);
	char *b = console_line_buff;
	while(*b != 0) b++;
	*b++ = ch;
	*b++ = 0;
}

static void
newline(void){
	char *curr_line = malloc(strlen(console_line_buff) + 1);
	strcpy(curr_line, console_line_buff);
	util_list_insert(console_text, curr_line);

	memset(console_line_buff, 0, 2048);
}

static void
activate_console(void){
	LOG_DEBUG("Console actived");
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	console_fadeout_alpha = 1.0;
	fadeout_task->is_active = 0;
	util_anim_reset_anim_task(cursor_task);
	console_isactive = 1;
}

static void
deactivate_console(void){
	LOG_DEBUG("Console deactived");
	SDL_EnableUNICODE(0);
	SDL_EnableKeyRepeat(0, 0);
	cursor_task->is_active = 0;
	util_anim_reset_anim_task(fadeout_task);
	console_isactive = 0;
}

static int 
keydown_callback(SDL_Event *event){
	if(console_isactive){
		if(event->key.keysym.sym == SDLK_F2 || event->key.keysym.sym == SDLK_ESCAPE){
			deactivate_console();
			return 1;
		}else if(event->key.keysym.sym == SDLK_BACKSPACE){
			backspace();
			return 1;
		}else if(event->key.keysym.sym == SDLK_RETURN){
			if(!isempty())
				newline();
			return 1;
		}else{
			if((event->key.keysym.unicode & 0xFF80) == 0){
				char ch = event->key.keysym.unicode & 0x7F;
				add_char(ch);
			}else{
				LOG_DEBUG("An international character");
			}
			return 1;
		}
	}else{
		if(event->key.keysym.sym == SDLK_F2){
			activate_console();
			return 1;
		}
	}
	return 0;
}

void
update_cursor(double scale_factor){
	static double a = 0;
	a += scale_factor;
	printf("%f\n", a);
	if(a >= 1){
		a = 0;
		if(draw_cursor) 
			draw_cursor = 0;
		else
			draw_cursor = 1;
	}
}

void
update_fadeout(double scale_factor){
	console_fadeout_alpha -= scale_factor;
	if(console_fadeout_alpha < 0)
		console_fadeout_alpha = 0;
}

void
console_init(void){
	keydown_handler = malloc(sizeof(event_handler_t));
	keydown_handler->type_filter = SDL_KEYDOWN;
	keydown_handler->callback = keydown_callback;
	event_add_event_handler(keydown_handler);

	cursor_task = util_anim_create(4, -1, 0, update_cursor);
	fadeout_task = util_anim_create(1, 1, 0, update_fadeout);

	console_text = util_list_create();

	newline();
}
STARTUP_PROC(console, 6, console_init)

void
console_draw(void){
	if(console_isactive){
		char buff[2050];
		strcpy(buff, "> ");
		strcat(buff, console_line_buff);
		if(draw_cursor){
			strcat(buff, "_");
		}
		hud_draw_string(5, 100, 12, 16, buff);
	}
	for(int i = 0; i < 5; i++){
		char *line = util_list_get(console_text, i);
		if(line != NULL)
			hud_draw_string_with_alpha(5, 100 - 16 * (i+1), 12, 16, console_isactive ? 1.0 : console_fadeout_alpha, line);
	}
}

int
console_active(void){
	return console_isactive;
}

