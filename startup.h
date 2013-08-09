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
