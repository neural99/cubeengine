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
	double forward[3];
	double up[3];
} camera_t;

SDLKey listen_keys[4] = {SDLK_w, SDLK_s, SDLK_a, SDLK_d};

static camera_t *camera;
static keypress_handler_t *camera_handlers[4];
static event_handler_t *camera_mousemove_handler;
static keypress_handler_t *wireframe_handler;

int handle_keypress(SDL_Event *e);
int handle_mousemove(SDL_Event *e);
int handle_wireframe_key(SDL_Event *e);
static void move_forward(void);
static void move_backward(void);
static void move_left(void);
static void move_right(void);

void
camera_create(void){
	camera_t *tmp = malloc(sizeof(camera_t));
	tmp->eye[0] = 0.0; tmp->eye[1] = 0.0; tmp->eye[2] = 0.0;
	tmp->forward[0] = 0.0; tmp->forward[1] = 0.0; tmp->forward[2] = 1.0;
	tmp->up[0] = 0.0; tmp->up[1] = 1.0; tmp->up[2] = 0.0;

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

	free(camera);
}

void
camera_load_perspective(void){
	glViewport(0, 0, 800, 800);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double aspect = 800 / 800;
	glOrtho(-aspect, aspect, -1, 1, -1, 1);
	gluPerspective(60.0, aspect, 0.01, 10000.0);
	
	glMatrixMode(GL_MODELVIEW);
}


int handle_wireframe_key(SDL_Event *e){
	static int iswireframe = 0;
	if(!iswireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	iswireframe = !iswireframe;
}

int 
handle_keypress(SDL_Event *e){
	printf("eye = (%f %f %f), forward= (%f %f %f), up = (%f %f %f)\n", camera->eye[0], camera->eye[1], camera->eye[2],
		       						 	   camera->forward[0], camera->forward[1], camera->forward[2],
									   camera->up[0], camera->up[1], camera->up[2]);
	SDLKey sym = e->key.keysym.sym;
	if(sym == SDLK_w)
		move_forward();
	else if(sym == SDLK_s)
		move_backward();
	else if(sym == SDLK_a)
		move_left();
	else if(sym == SDLK_d)
		move_right();
	return 0;
}

int 
handle_mousemove(SDL_Event *e){
	printf("eye = (%f %f %f), forward= (%f %f %f), up = (%f %f %f)\n", camera->eye[0], camera->eye[1], camera->eye[2],
		       						 	   camera->forward[0], camera->forward[1], camera->forward[2],
									   camera->up[0], camera->up[1], camera->up[2]);
	int xrel = e->motion.xrel;
	int yrel = e->motion.yrel;

	double angle1 = 0;
	double angle2 = 0;
	if(xrel != 0)
		angle1 = 2 * PI * 0.01 / xrel;
	if(yrel != 0)
		angle2 = 2 * PI * 0.01 / yrel;
	if(angle1 != 0)
		rotatearoundYaxis(camera->forward, -angle1);
	if(angle2 != 0)
		rotatearoundXaxis(camera->forward, angle2);	

	normalize(camera->forward);

	return 0;
}

void
camera_load_modelview(void){
	double center[3];
	center[0] = camera->eye[0] + camera->forward[0];
	center[1] = camera->eye[1] + camera->forward[1];
	center[2] = camera->eye[2] + camera->forward[2];

	gluLookAt(camera->eye[0], camera->eye[1], camera->eye[2], 
		  center[0], center[1], center[2],
		  camera->up[0], camera->up[1], camera->up[2]);
}

static void
move_forward(void){
	camera->eye[0] += camera->forward[0] * STEP_FACTOR;
	camera->eye[1] += camera->forward[1] * STEP_FACTOR;
	camera->eye[2] += camera->forward[2] * STEP_FACTOR;
}

static void 
move_backward(void){
	camera->eye[0] -= camera->forward[0] * STEP_FACTOR;
	camera->eye[1] -= camera->forward[1] * STEP_FACTOR;
	camera->eye[2] -= camera->forward[2] * STEP_FACTOR;
}

static void 
move_left(void){
	double n[3];
	crossproduct(n, camera->up, camera->forward);
	normalize(n);

	//printf("n=(%f %f %f)\n", n[0], n[1], n[2]);

	camera->eye[0] += n[0] * STEP_FACTOR; 
	camera->eye[1] += n[1] * STEP_FACTOR; 
	camera->eye[2] += n[2] * STEP_FACTOR;
}

static void
move_right(void){
	double n[3];
	crossproduct(n, camera->forward, camera->up);
	normalize(n);

	camera->eye[0] += n[0] * STEP_FACTOR; 
	camera->eye[1] += n[1] * STEP_FACTOR; 
	camera->eye[2] += n[2] * STEP_FACTOR;
}

void
camera_move(double x, double y, double z){
	camera->eye[0] = x; camera->eye[1] = y; camera->eye[2] = z;

}
