#include <math.h>
#include <stdio.h>
#include <SDL/sdl.h>
#include <GL/gl.h>
#include <GL/glu.h>

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

int handle_keydown(SDL_Event *e);
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

	for(int i = 0; i < 4; i++){
		camera_handlers[i] = malloc(sizeof(keypress_handler_t));
		camera_handlers[i]->sym = listen_keys[i];
		camera_handlers[i]->repeat_interval = 100;
		camera_handlers[i]->timer = 0;
		camera_handlers[i]->callback = handle_keydown;
		event_add_keypress_handler(camera_handlers[i]);
	}
}

void
camera_free(void){
	for(int i = 0; i < 4; i++){
		event_remove_keypress_handler(camera_handlers[i]);
		free(camera_handlers[i]);
	}

	free(camera);
}

void
camera_load_perspective(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 800 / 400, 0.0001, 10000.0);
	
	glMatrixMode(GL_MODELVIEW);
}


int 
handle_keydown(SDL_Event *e){
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
crossproduct(double a[3], double b[3], double c[3]){
	a[0] = b[1] * c[2] - b[2] * c[1];
	a[1] = b[2] * c[0] - b[0] * c[2];
	a[2] = b[0] * c[1] - b[1] * c[0];
}

static double
length(double v[3]){
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

static void
normalize(double v[3]){
	double len = length(v);
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
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


