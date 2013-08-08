#ifndef __STARTUP_H__
#define __STARTUP_H__

typedef struct startup_task_s {
	char *name;
	int prio;
	void (*initfunc)(void);
} startup_task_t;

#define STARTUP_PROC(module_name, prio, func) startup_task_t module_name ## _task = { #module_name, prio, func }; 
#define ADD_PROC(module_name, a, b) extern startup_task_t module_name ## _task ; util_list_add(startup_list, &module_name ## _task); 

void startup_engine(void);

#endif
