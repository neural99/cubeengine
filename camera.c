#include <math.h>
#include <stdio.h>
#include <SDL/sdl.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "util.h"
#include "camera.h"
#include "event.h"

#define STEP_FACTOR 0.1

typedef struct camera_s {
	double eye[3];

	quaternion_t forward;
	quaternion_t up;
} camera_t;

SDLKey listen_keys[4] = {SDLK_w, SDLK_s, SDLK_a, SDLK_d};

static camera_t *camera;
static keypress_handler_t *camera_handlers[4];
static event_handler_t *camera_mousemove_handler;
static keypress_handler_t *wireframe_handler;

static anim_task_t *animation_tasks[4];

static int handle_keypress(SDL_Event *e);
static int handle_mousemove(SDL_Event *e);
static int handle_wireframe_key(SDL_Event *e);
static void move_forward(double upf);
static void move_backward(double upf);
static void move_left(double upf);
static void move_right(double upf);
static void add_anim_tasks(void);
static void remove_anim_tasks(void);

void
camera_create(void){ camera_t *tmp = malloc(sizeof(camera_t));
	tmp->eye[0] = 0.0; tmp->eye[1] = 0.0; tmp->eye[2] = 0.0;
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
}

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

	free(camera);
}

void
camera_load_perspective(void){
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double aspect = WINDOW_WIDTH / WINDOW_HEIGHT;
	glOrtho(-aspect, aspect, -1, 1, -1, 1);
	gluPerspective(60.0, aspect, 0.01, 10000.0);
	
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
	printf("eye = (%f %f %f), forward= (%f %f %f %f), up = (%f %f %f %f)\n", camera->eye[0], camera->eye[1], camera->eye[2],
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
	double rot_axis[3];
	double y_axis[3];

	y_axis[0] = 0;
	y_axis[1] = 1;
	y_axis[2] = 0;

	forward_v[0] = camera->forward.x;
	forward_v[1] = camera->forward.y;
	forward_v[2] = camera->forward.z;
	up_v[0] = camera->up.x;
	up_v[1] = camera->up.y;
	up_v[2] = camera->up.z;

	crossproduct(rot_axis, forward_v, up_v);

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
		quad_rotate(&rot1, -angle1, y_axis);
	}
	if(angle2 != 0){
		quad_rotate(&rot2, -angle2, rot_axis);
	}
	quad_mult(&composite_rot, &rot1, &rot2);
	quad_normalize(&composite_rot);

	quaternion_t new_forward = camera->forward;
	quaternion_t yaxis;
	yaxis.x = 0.0;
	yaxis.y = 1.0;
	yaxis.z = 0.0;
	yaxis.w = 1.0;
	quaternion_t negyaxis = yaxis;
	quad_conjugate(&negyaxis);

	quad_applyrotation(&new_forward, &composite_rot);
	if(quad_diff(&yaxis, &new_forward) > 0.1 && quad_diff(&negyaxis, &new_forward) > 0.1){
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

