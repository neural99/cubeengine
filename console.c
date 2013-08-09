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

static console_command_arg_t*
parse_arg(char *str){
	int items; 

	console_command_arg_t *arg = malloc(sizeof(console_command_arg_t));

	/* Float */
	items = sscanf(str, "%f", &arg->floatval);
	if(items == 1){
		arg->type = ARG_FLOAT;
		return arg;
	}

	/* Integer */
	items = sscanf(str, "%d", &arg->intval);
	if(items == 1){
		arg->type = ARG_INT;
		return arg;
	}

	/* Bool */
	int tru = strcmp(str, "true") == 0 || strcmp(str, "True") == 0;
	int fal = strcmp(str, "false") == 0 || strcmp(str, "False") == 0;
	if(tru || fal){
		arg->type = ARG_BOOL;
		return arg;
	}

	/* String */
	arg->strval = malloc(strlen(str) + 1);
	strcpy(arg->strval, str);
	arg->type = ARG_STRING;
	return arg;
}

static void
execute_cmd_if_signature_matches(console_command_t *cmd, linked_list_t *args){
	int len;
	len = util_list_size(args);

	/* Wrong number of args? */
	if(len >= MAX_ARGS)
		FATAL_ERROR("Invocation of command %s failed. Number of args is bigger than MAX_ARGS", cmd->name);
	if(len != cmd->n_args){
		LOG_DEBUG("Command %s called with wrong number of arguments. Got %d but %d was expected", cmd->name, len, cmd->n_args); 
		console_output_line("Wrong number of arguments");
		return;
	}
	
	/* Check arg types */
	for(int i = 0; i < len; i++){
		console_command_arg_t *arg_type;
	        arg_type = util_list_get(args, i);
		if(cmd->arg_types[i] != arg_type->type){
			console_output_line("Wrong cmd signature");
			return;
		}
	}

	/* Invoke command callback */
	char *res = cmd->execute(args);
	console_output_line(res);
}

static int
maybe_execute_cmd(char *name, linked_list_t *args){
	int found = 0;
	int len = util_list_size(console_command_list);
	for(int i = 0; i < len; i++){
		console_command_t *cmd = util_list_get(console_command_list, i);
		printf("cmd->name = %s, name = %s\n\n", cmd->name, name);
		if(strcmp(cmd->name, name) == 0){
			execute_cmd_if_signature_matches(cmd, args);
			found = 1;
			break;
		}
	}
	return found;
}

static void
arg_free(void *data){
	console_command_arg_t *arg = data;
	if(arg->type == ARG_STRING)
		free(arg->strval);
	free(arg);
}

static int 
parse(char *line){
	linked_list_t *strs;
	linked_list_t *args;
	char *name;
	char *str;
	int n_strs; 

	strs = split_on_whitespace(line);
	n_strs = util_list_size(strs);
	args = util_list_create();

	/* Abort if line consists of whitespace only */
	if(n_strs == 0)
		return 0;

	name = util_list_get(strs, 0);
	for(int i = 1; i < n_strs ; i++){
		str = util_list_get(strs, i);
		console_command_arg_t *arg = parse_arg(str);
		util_list_add(args, arg);
	}

	int res = maybe_execute_cmd(name, args);

	free(name);
	util_list_free_custom(args, arg_free);
	util_list_free_data(strs);

	return res;
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
	printf("new line %s\n\n\n", console_line_buff);
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
		if(event->key.keysym.sym == SDLK_ESCAPE){
			deactivate_console();
			return 1;
		}else if(event->key.keysym.sym == SDLK_BACKSPACE){
			backspace();
			return 1;
		}else if(event->key.keysym.sym == SDLK_RETURN){
			if(!isempty())
				newline();
			else
				deactivate_console();
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
		if(event->key.keysym.sym == SDLK_RETURN){
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

