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

#include <math.h>
#include <stdio.h>
#include <SDL/sdl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "util.h"
#include "camera.h"
#include "event.h"
#include "console.h"
#include "startup.h"

#define STEP_FACTOR 0.1

camera_t *camera;

SDLKey listen_keys[4] = {SDLK_w, SDLK_s, SDLK_a, SDLK_d};

static keypress_handler_t *camera_handlers[4];
static event_handler_t *camera_mousemove_handler;
static keypress_handler_t *wireframe_handler;

static anim_task_t *animation_tasks[4];

static console_command_t *pos_print_cmd;
static console_command_t *pos_move_cmd;

static int handle_keypress(SDL_Event *e);
static int handle_mousemove(SDL_Event *e);
static int handle_wireframe_key(SDL_Event *e);
static void move_forward(double upf);
static void move_backward(double upf);
static void move_left(double upf);
static void move_right(double upf);
static void add_anim_tasks(void);
static void remove_anim_tasks(void);
static void add_console_cmds(void);
static void remove_console_cmds(void);

void
camera_create(void){ camera_t *tmp = malloc(sizeof(camera_t));
	tmp->eye[0] = 0.0; tmp->eye[1] = 0.0; tmp->eye[2] = -10.0;
	tmp->forward.x = 0.0; tmp->forward.y = 0.0; tmp->forward.z = 1.0; tmp->forward.w = 1.0; 
	tmp->up.x = 0.0; tmp->up.y = 1.0; tmp->up.z = 0.0; tmp->up.w = 1.0;
	//tmp->rot.x = 0.0; tmp->rot.y = 0.0; tmp->rot.z = 0.0; tmp->rot.w = 1.0; 

	camera = tmp;

	/* Add keypress callbacks */
	for(int i = 0; i < 4; i++){
		camera_handlers[i] = malloc(sizeof(keypress_handler_t));
		camera_handlers[i]->sym = listen_keys[i];
		camera_handlers[i]->repeat_interval = 100;
		camera_handlers[i]->timer = 0;
		camera_handlers[i]->callback = handle_keypress;
		event_add_keypress_handler(camera_handlers[i]);
	}
	/* Add mousemove callback */
	camera_mousemove_handler = malloc(sizeof(event_handler_t));
	camera_mousemove_handler->type_filter = SDL_MOUSEMOTION;
	camera_mousemove_handler->callback = handle_mousemove;
	event_add_event_handler(camera_mousemove_handler);

	/* Add wireframe callback */
	wireframe_handler = malloc(sizeof(keypress_handler_t));
	wireframe_handler->sym = SDLK_u;
	wireframe_handler->repeat_interval = 1000;
	wireframe_handler->timer = 0;
	wireframe_handler->callback = handle_wireframe_key;
	event_add_keypress_handler(wireframe_handler);

	add_anim_tasks();
	add_console_cmds();
}
STARTUP_PROC(camera, 3, camera_create)

void
camera_free(void){
	for(int i = 0; i < 4; i++){
		event_remove_keypress_handler(camera_handlers[i]);
		free(camera_handlers[i]);
	}
	event_remove_event_handler(camera_mousemove_handler);
	free(camera_mousemove_handler);
	event_remove_keypress_handler(wireframe_handler);
	free(wireframe_handler);

	remove_anim_tasks();
	remove_console_cmds();

	free(camera);
}

void
camera_load_perspective(void){
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double aspect = WINDOW_WIDTH / WINDOW_HEIGHT;
	gluPerspective(60.0, aspect, NEAR_PLANE, FAR_PLANE);
	
	glMatrixMode(GL_MODELVIEW);
}


static int
handle_wireframe_key(SDL_Event *e){
	static int iswireframe = 0;
	if(!iswireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	iswireframe = !iswireframe;

	return 0;
}

static int 
handle_keypress(SDL_Event *e){
	LOG_DEBUG("eye = (%f %f %f), forward= (%f %f %f %f), up = (%f %f %f %f)\n", camera->eye[0], camera->eye[1], camera->eye[2],
		       				     camera->forward.x, camera->forward.y, camera->forward.z, camera->forward.w,
					     	   camera->up.x, camera->up.y, camera->up.z, camera->up.w);
	SDLKey sym = e->key.keysym.sym;
	if(sym == SDLK_w)
		util_anim_reset_anim_task(animation_tasks[0]);
	else if(sym == SDLK_s)
		util_anim_reset_anim_task(animation_tasks[1]);
	else if(sym == SDLK_a)
		util_anim_reset_anim_task(animation_tasks[2]);
	else if(sym == SDLK_d)
		util_anim_reset_anim_task(animation_tasks[3]);
	return 0;
}

static int 
handle_mousemove(SDL_Event *e){
	int xrel = e->motion.xrel;
	int yrel = e->motion.yrel;

	double forward_v[3];
	double up_v[3];
	double rot_axis_v[3];

	quaternion_t y_axis;
	quaternion_t rot_axis;

	forward_v[0] = camera->forward.x;
	forward_v[1] = camera->forward.y;
	forward_v[2] = camera->forward.z;
	up_v[0] = camera->up.x;
	up_v[1] = camera->up.y;
	up_v[2] = camera->up.z;

	crossproduct(rot_axis_v, forward_v, up_v);

	rot_axis.x = rot_axis_v[0]; rot_axis.y = rot_axis_v[1]; rot_axis.z = rot_axis_v[2]; rot_axis.w = 1;
	y_axis.x = 0; y_axis.y = 1; y_axis.z = 0; y_axis.w = 1;

	quaternion_t rot1;
	rot1.x = 0; rot1.y = 0; rot1.y = 0; rot1.z = 0; rot1.w = 1.0;
	quaternion_t rot2;
	rot2.x = 0; rot2.y = 0; rot2.y = 0; rot2.z = 0; rot2.w = 1.0;
	quaternion_t composite_rot;

	double angle1 = 0;
	double angle2 = 0;
	if(xrel != 0)
		angle1 = 2 * PI * 0.01 / xrel;
	if(yrel != 0)
		angle2 = 2 * PI * 0.01 / yrel;
	if(angle1 != 0){
		quad_rotate(&rot1, -angle1, &y_axis);
	}
	if(angle2 != 0){
		quad_rotate(&rot2, -angle2, &rot_axis);
	}
	quad_mult(&composite_rot, &rot1, &rot2);
	quad_normalize(&composite_rot);

	quaternion_t new_forward = camera->forward;
	quaternion_t neg_y_axis = y_axis;
	quad_conjugate(&neg_y_axis);

	/* Avoid gimbal lock by aborting if we are too close to the y_axis */
	quad_applyrotation(&new_forward, &composite_rot);
	if(quad_diff(&y_axis, &new_forward) > 0.1 && quad_diff(&neg_y_axis, &new_forward) > 0.1){
		camera->forward = new_forward;
		quad_applyrotation(&camera->up, &composite_rot);
	}

	quad_normalize(&camera->forward);
	quad_normalize(&camera->up);

	return 0;
}

void
camera_load_modelview(void){
	double center[3];
	center[0] = camera->eye[0] + camera->forward.x;
	center[1] = camera->eye[1] + camera->forward.y;
	center[2] = camera->eye[2] + camera->forward.z;

	gluLookAt(camera->eye[0], camera->eye[1], camera->eye[2], 
		  center[0], center[1], center[2],
		  camera->up.x, camera->up.y, camera->up.z);
}

static void
move_forward(double factor){
	camera->eye[0] += camera->forward.x * factor;
	camera->eye[1] += camera->forward.y * factor;
	camera->eye[2] += camera->forward.z * factor;
}

static void 
move_backward(double factor){
	camera->eye[0] -= camera->forward.x * factor;
	camera->eye[1] -= camera->forward.y * factor;
	camera->eye[2] -= camera->forward.z * factor;
}

static void 
move_left(double factor){
	double n[3];
	double forward_v[3];
	double up_v[3];

	forward_v[0] = camera->forward.x;
	forward_v[1] = camera->forward.y;
	forward_v[2] = camera->forward.z;
	up_v[0] = camera->up.x;
	up_v[1] = camera->up.y;
	up_v[2] = camera->up.z;

	crossproduct(n, up_v, forward_v);
	normalize(n);

	//printf("n=(%f %f %f)\n", n[0], n[1], n[2]);

	camera->eye[0] += n[0] * factor; 
	camera->eye[1] += n[1] * factor; 
	camera->eye[2] += n[2] * factor;
}

static void
move_right(double factor){
	double n[3];
	double forward_v[3];
	double up_v[3];

	forward_v[0] = camera->forward.x;
	forward_v[1] = camera->forward.y;
	forward_v[2] = camera->forward.z;
	up_v[0] = camera->up.x;
	up_v[1] = camera->up.y;
	up_v[2] = camera->up.z;

	crossproduct(n, forward_v, up_v);
	normalize(n);

	camera->eye[0] += n[0] * factor; 
	camera->eye[1] += n[1] * factor; 
	camera->eye[2] += n[2] * factor;
}

void
camera_move(double x, double y, double z){
	camera->eye[0] = x; camera->eye[1] = y; camera->eye[2] = z;

}

static void
add_anim_tasks(void){
	animation_tasks[0] = util_anim_create(10.0, 1.0, 0, move_forward);
	animation_tasks[1] = util_anim_create(10.0, 1.0, 0, move_backward);
	animation_tasks[2] = util_anim_create(10.0, 1.0, 0, move_left);
	animation_tasks[3] = util_anim_create(10.0, 1.0, 0, move_right);
}

static void
remove_anim_tasks(void){
	for(int i = 0; i < 4; i++){
		util_anim_remove_anim_task(animation_tasks[i]);
		free(animation_tasks[i]);
	}
}

static char* 
pos_print_execute(void **args){
	char *out = malloc(200);
	snprintf(out, 200, "pos:(%f,%f,%f)", camera->eye[0], camera->eye[1], camera->eye[2]);
	return out;
}

static char*
pos_move_execute(void **args){
	float x, y, z;
	x = *((float*)args[0]);
	y = *((float*)args[1]);
	z = *((float*)args[1]);
	
	camera_move(x, y, z);

	char *out = malloc(20);
	snprintf(out, 20, "Pos changed");
	return out;
}

static void
add_console_cmds(void){
	pos_print_cmd = malloc(sizeof(console_command_t));
	strcpy(pos_print_cmd->name, "pos");
	pos_print_cmd->n_args = 0;
	pos_print_cmd->execute = pos_print_execute;
	console_add_command(pos_print_cmd);

	pos_move_cmd = malloc(sizeof(console_command_t));
	strcpy(pos_move_cmd->name, "pos");
	pos_move_cmd->n_args = 3;
	memset(pos_move_cmd->arg_types, 0, sizeof(console_command_arg_type_t) * MAX_ARGS);
	pos_move_cmd->arg_types[0] = ARG_FLOAT; pos_move_cmd->arg_types[1] = ARG_FLOAT; pos_move_cmd->arg_types[2] = ARG_FLOAT;
	console_add_command(pos_move_cmd);
}

static void
remove_console_cmds(void){
	console_remove_command(pos_print_cmd);
	console_remove_command(pos_move_cmd);
	free(pos_print_cmd);
	free(pos_move_cmd);
}
