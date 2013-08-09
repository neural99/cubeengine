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

static linked_list_t *console_command_list;

static event_handler_t *keydown_handler;
static anim_task_t *cursor_task;
static anim_task_t *fadeout_task;

void
console_output_line(char *str){
	util_list_insert(console_text, str);
	LOG_DEBUG("Console %s", str);
}

static void 
read_whitespace(char **p){
	char *c = *p;
	while(*c != 0 && *c == ' ')
		c++;
	*p = c;
}

static linked_list_t*
split_on_whitespace(char *p){
	linked_list_t *str_list = util_list_create();

	char buff[2048];
	while(*p != 0){
		memset(buff, 0, 2048);

		read_whitespace(&p);
		int i = 0;
		while(*p != 0 && *p != ' ') 
			buff[i++] = *p++;
		/* Copy to heap */
		char *str = malloc(i + 2);
		strcpy(str, buff);
		
		util_list_add(str_list, str);
	}
	return str_list;
}

static console_command_arg_type_t 
parse_arg(char *str, void **out_data){
	int items; 

	float floatval;
	int intval;

	items = sscanf(str, "%f", &floatval);
	if(items == 1){
		float *tmp = malloc(sizeof(float));
		*tmp = floatval;
		*out_data = tmp;
		return ARG_FLOAT;
	}

	items = sscanf(str, "%d", &intval);
	if(items == 1){
		int *tmp = malloc(sizeof(int));
		*tmp = intval;
		*out_data = tmp;
		return ARG_INT;
	}

	/* Bool */
	int tru = strcmp(str, "true") == 0 || strcmp(str, "True") == 0;
	int fal = strcmp(str, "false") == 0 || strcmp(str, "False") == 0;
	if(tru || fal){
		int *tmp = malloc(sizeof(int));
		*tmp = tru ? 1 : 0;
		*out_data = tmp;
		return ARG_BOOL;
	}

	/* String */
	*out_data = str;
	return ARG_STRING;
}

static int
execute_cmd(char *name, void**args){
	linked_list_elm_t *elm;
	int found = 0;
	elm = console_command_list->head;
	while(elm != NULL){
		console_command_t *cmd = elm->data;
		if(strcmp(cmd->name, name) == 0){
			char * res = cmd->execute(args);
			console_output_line(res);
			found = 1;
			break;
		}
		elm = elm->next;
	}
	return found;
}

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
	char *b = console_line_buff;
	while(*b != 0) b++;
	*b++ = ch;
	*b++ = 0;
}

static void
newline(void){
	int res = parse(console_line_buff);
	if(!res){
		/* Insert into console log if not cmd */
		char *curr_line = malloc(strlen(console_line_buff) + 1);
		strcpy(curr_line, console_line_buff);
		util_list_insert(console_text, curr_line);
	}

	
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
	console_command_list = util_list_create();

	newline();
}
STARTUP_PROC(console, 2, console_init)

void
console_cleanup(void){
	event_remove_event_handler(keydown_handler);
	free(keydown_handler);

	util_anim_remove_anim_task(cursor_task);
	util_anim_remove_anim_task(fadeout_task);
	util_list_free_data(console_text);
}

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

void
console_add_command(console_command_t *cmd){
	util_list_add(console_command_list, cmd);
}

void
console_remove_command(console_command_t *cmd){
	util_list_remove(console_command_list, cmd);
}

