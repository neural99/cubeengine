#include <SDL/sdl.h>
#include <GL/gl.h>
#include <stdio.h>

#include "hud.h"
#include "util.h"
#include "event.h"
#include "camera.h"
#include "world.h"

static SDL_Surface *screen;
static int user_pressed_quit;
static event_handler_t *quit_handler;
static tile_t *cross_tile;

void load_hud(void);

void
init_graphics(void){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	

	screen = SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	load_hud();

	camera_create();
	camera_move(0, 0, -10);
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
load_hud(void){
	cross_tile = hud_load_single_tile("cross.bmp", 0xff, 0x00, 0xff);
}

void
draw_hud(void){
	int x = (800 + cross_tile->w)/2;
	int y = (800 + cross_tile->h)/2;
	hud_draw_tile(x, y, -1, -1, cross_tile);

	hud_draw_string(x, y+100, 60, 80, "111");
}


void
draw_frame(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_load_perspective();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera_load_modelview();

	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 8; j++)
			for(int k = 0; k < 8; k++)
				renderblock(i,j,k);

	draw_hud();
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
	hud_init();
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
	hud_cleanup();
	event_cleanup();
}

int
main(int argc, char *argv[]){
	game_loop();

	return 0;
}
