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

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define MAX_ARGS 10

typedef enum {
	ARG_NONE,
	ARG_INT,
	ARG_BOOL,
	ARG_FLOAT,
	ARG_STRING
} console_command_arg_type_t;

typedef struct console_command_s {
	char name[30];
	int n_args;
	console_command_arg_type_t arg_types[MAX_ARGS];
	char* (*execute)(void **args);
} console_command_t;

void console_add_command(console_command_t *cmd);
void console_remove_command(console_command_t *cmd);

void console_init(void);
void console_cleanup(void);
void console_draw(void);
int console_active(void);
void console_output_line(char *str); /* Ownership of str is passed on to console */

#endif
