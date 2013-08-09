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
