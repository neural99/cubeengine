#include <SDL/sdl.h>
#include <GL/glee.h>
#include <stdio.h>

#include "hud.h"
#include "util.h"
#include "event.h"
#include "camera.h"
#include "world.h"

static SDL_Surface *screen;
static int user_pressed_quit;
static event_handler_t *quit_handler;
static event_handler_t *mouse_down_handler;
static Uint32 last_fps = 0;
static skybox_t *skybox;

void load_world(void);

void
show_info_log(GLuint object, GLEEPFNGLGETSHADERIVPROC glGet__iv, GLEEPFNGLGETPROGRAMINFOLOGPROC glGet__InfoLog){
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

char*
file_contents(char *filename){
	FILE *f = fopen(filename, "r");
	if(f == NULL)
		FATAL_ERROR("Could not open file %s", filename);
	char *buff;
	buff = malloc(2048);
	memset(buff, 0, 2048);
	fread(buff, 1, 2048, f);
	return buff;
}

GLuint
make_shader(GLenum type, char *filename){
    GLchar *source = file_contents(filename);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, NULL);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
	    
GLuint
make_program(GLuint vertex_shader, GLuint fragment_shader){
    GLint program_ok;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}
    

void
setup_phong(void){
	GLuint frag = make_shader(GL_FRAGMENT_SHADER, "frag.glsl");
	GLuint vert = make_shader(GL_VERTEX_SHADER, "vert.glsl");
	GLuint program = make_program(vert, frag);
	printf("program=%d\n", program);
}

void
init_graphics(void){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	

	screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_OPENGL);

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	glShadeModel(GL_FLAT);

	//setup_phong();

	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);

	skybox = skybox_create();
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

int
mouse_callback(SDL_Event *e){
	if(e->button.type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_RIGHT){
		if(hud_selected_block[0] != -1)
			chunk_remove_block(hud_selected_chunk, hud_selected_block[0], hud_selected_block[1], hud_selected_block[2]);
	}
	return 0;
}

void 
setup_event_handlers(void){
	quit_handler = malloc(sizeof(event_handler_t));
	quit_handler->type_filter = SDL_KEYDOWN;
	quit_handler->callback = quit_callback;
	event_add_event_handler(quit_handler);

	mouse_down_handler = malloc(sizeof(event_handler_t));
	mouse_down_handler->type_filter = SDL_MOUSEBUTTONDOWN;
	mouse_down_handler->callback = mouse_callback;
	event_add_event_handler(mouse_down_handler);
}

void
cleanup_event_handlers(void){
	event_remove_event_handler(quit_handler);
	event_remove_event_handler(mouse_down_handler);
	free(quit_handler);
	free(mouse_down_handler);
}

void
draw_hud(void){
	hud_draw_selection_cube();
	hud_draw_selection_cross();

	/* draw fps and render info */
 	char buff[100];
	memset(buff, 0, 100);
	snprintf(buff, 100, "%u", last_fps);
	hud_draw_string(5, 565, 24, 32, buff);
	memset(buff, 0, 100);
	snprintf(buff, 100, "chunks:%d blocks:%d trigs:%d", chunkmanager_nchunks(), chunkmanager_activeblocks(), chunkmanager_ntrigs());
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
	chunk_render(chunk1);
	chunk_render(chunk2);
	*/
	skybox_render(skybox);
	chunkmanager_render_world();
	//renderblock(0, 0, 5);
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
	chunkmanager_update();
}

void
game_loop(void){
	user_pressed_quit = 0;

	init_graphics();
	startup_engine();
	//hud_init();
	//camera_create();
	setup_event_handlers();
	//textureset_init();
	load_world();

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

	chunkmanager_free();
	cleanup_graphics();
	cleanup_event_handlers();
	hud_cleanup();
	textureset_free();
	event_cleanup();
}

void
load_world(void){
	world_file_t world;
	world.path = "test.wrl";
	if(world_open(&world) < 0)
		FATAL_ERROR("Couldnt open world file");
	chunkmanager_init(&world);
	chunkmanager_rebuild();
}

int
main(int argc, char *argv[]){
	game_loop();

	return 0;
}
