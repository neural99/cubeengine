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
static Uint32 last_fps = 0;
static chunk_t *chunk1, *chunk2;

void load_hud(void);

void
init_graphics(void){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL);

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
	hud_init();
	cross_tile = hud_load_single_tile("cross.bmp", 0xff, 0x00, 0xff, GL_LINEAR);
}

void
draw_hud(void){
	int x = (WINDOW_WIDTH - cross_tile->w)/2;
	int y = (WINDOW_HEIGHT + cross_tile->h)/2;
	hud_draw_tile(x, y, -1, -1, cross_tile);

	/* draw fps */
 	char buff[100];
	memset(buff, 0, 100);
	snprintf(buff, 100, "%u", last_fps);
	hud_draw_string(5, 565, 24, 32, buff);

	int trigs = chunk1->mesh->n_trigs + chunk2->mesh->n_trigs;
	int verts = chunk1->mesh->n_verticies + chunk2->mesh->n_verticies;
	memset(buff, 0, 100);
	snprintf(buff, 100, "trigs:%d vert:%d", trigs, verts);
	hud_draw_string(5, 550, 12, 16, buff);
}


void
draw_frame(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_load_perspective();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	camera_load_modelview();

	/*
	for(int i = 0; i < 8; i++)
		for(int j = 0; j < 8; j++)
			for(int k = 0; k < 8; k++)
				renderblock(i,j,k);
	*/
	chunk_render(chunk1);
	chunk_render(chunk2);

	draw_hud();
}

void
calc_fps(Uint32 now){
	static Uint32 frames = 0;
	static Uint32 ticks_last_updated = 0;
	if(ticks_last_updated == 0)
		ticks_last_updated = now;
	if(now - ticks_last_updated > 1e3){
		last_fps = frames;
		frames = 0;
		ticks_last_updated = now;
	}else{
		frames++;
	}
}

void
do_logic(Uint32 diff){
	util_anim_update(diff);
}

void
game_loop(void){
	user_pressed_quit = 0;

	event_init();
	setup_event_handlers();
	init_graphics();

	chunk1 = chunk_create();
	chunk2 = chunk_create();
	chunk2->pos[0] = 17;

	Uint32 prev_ticks, curr_ticks;
	curr_ticks = SDL_GetTicks();
	while(!user_pressed_quit){
		handle_SDL_events();
		draw_frame();

		SDL_GL_SwapBuffers();

		prev_ticks = curr_ticks;
		curr_ticks = SDL_GetTicks();
		calc_fps(curr_ticks);
		do_logic(curr_ticks - prev_ticks);
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
