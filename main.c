#include <SDL/sdl.h>
#include <GL/gl.h>
#include <stdio.h>

#include "event.h"
#include "camera.h"

static SDL_Surface *screen;
static int user_pressed_quit;
static event_handler_t *quit_handler;

void
init_graphics(void){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	

	screen = SDL_SetVideoMode(800, 600, 24, SDL_OPENGL);

	camera_create();
}

void
cleanup_graphics(void){
	camera_free();

	SDL_FreeSurface(screen);
	SDL_Quit();
}

void
handle_SDL_events(void){
	SDL_Event ev;
	int res;

	res = SDL_PollEvent(&ev);
	if(res == 1) 
		event_dispatch(&ev);
}

int
quit_callback(SDL_Event *e){
	if(e->key.keysym.sym == SDLK_q)
		user_pressed_quit = 1;
	return 0;
}

void 
setup_event_handlers(void){
	quit_handler = malloc(sizeof(event_handler_t));
	quit_handler->type_filter = SDL_KEYDOWN;
	quit_handler->callback = quit_callback;
	event_add_event_handler(quit_handler);
}

void
cleanup_event_handlers(void){
	event_remove_event_handler(quit_handler);
	free(quit_handler);
}

void
draw_frame(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_load_perspective();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera_load_modelview();

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 2.0f, 1.0f);
	glEnd();
}

void
display_fps(Uint32 diff){
	if (diff != 0){
		Uint32 fps = 1000/diff;
	}
}

void
game_loop(void){
	user_pressed_quit = 0;

	event_init();
	setup_event_handlers();
	init_graphics();

	Uint32 prev_ticks, curr_ticks;
	curr_ticks = SDL_GetTicks();
	while(!user_pressed_quit){
		handle_SDL_events();
		draw_frame();

		prev_ticks = curr_ticks;
		curr_ticks = SDL_GetTicks();
		display_fps(curr_ticks - prev_ticks);

		SDL_GL_SwapBuffers();
	}

	cleanup_graphics();
	cleanup_event_handlers();
	event_cleanup();
}

int
main(int argc, char *argv[]){
	game_loop();

	return 0;
}
