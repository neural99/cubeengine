#include <stdio.h>
#include <math.h>
#include <SDL/sdl.h>
#include <GL/glee.h>
#include <GL/glu.h>

#include "util.h"
#include "hud.h"
#include "world.h"
#include "camera.h"

static tile_t *cross_tile;
static tileset_t *font_tileset;
static int font_characters[][2] = {{ '0', 16 },
				   { '1', 17 },
				   { '2', 18 },
				   { '3', 19 },
				   { '4', 20 },
				   { '5', 21 },
				   { '6', 22 },
				   { '7', 23 },
				   { '8', 24 },
				   { '9', 25 },
				   { 'A', 33 },
				   { 'a', 33 },
				   { 'B', 34 },
				   { 'b', 34 },
				   { 'C', 35 },
				   { 'c', 35 },
				   { 'D', 36 },
				   { 'd', 36 },
				   { 'E', 37 },
				   { 'e', 37 },
				   { 'F', 38 },
				   { 'f', 38 },
				   { 'G', 39 },
				   { 'g', 39 },
				   { 'H', 40 },
				   { 'h', 40 },
				   { 'I', 41 },
				   { 'i', 41 },
				   { 'J', 42 },
				   { 'j', 42 },
				   { 'K', 43 },
				   { 'k', 43 },
				   { 'L', 44 },
				   { 'l', 44 },
				   { 'M', 45 },
				   { 'm', 45 },
				   { 'N', 46 },
				   { 'n', 46 },
				   { 'O', 47 },
				   { 'o', 47 },
				   { 'P', 48 },
				   { 'p', 48 },
				   { 'Q', 49 },
				   { 'q', 49 },
				   { 'R', 50 },
				   { 'r', 50 },
				   { 'S', 51 },
				   { 's', 51 },
				   { 'T', 52 },
				   { 't', 52 },
				   { 'U', 53 },
				   { 'u', 53 },
				   { 'V', 54 },
				   { 'v', 54 },
				   { 'W', 55 },
				   { 'w', 55 },
				   { 'X', 56 },
				   { 'x', 56 },
				   { 'Y', 57 },
				   { 'y', 57 },
				   { 'Z', 58 },
				   { 'z', 58 },
				   { ':', 26 },
				   { ' ', 0  }};

static void draw_quad_with_texture(int x, int y, int width, int height, GLuint texId);

void 
hud_init(void){
	font_tileset = hud_load_tileset("font.bmp", 6, 8, 16, 6, 1, 0xFF, 0x00, 0x00, GL_NEAREST);
	cross_tile = hud_load_single_tile("cross.bmp", 0xff, 0x00, 0xff, GL_NEAREST);
}

void
hud_cleanup(void){
	hud_unload_tileset(font_tileset);
}

void
hud_unload_tileset(tileset_t *tileset){
	glDeleteTextures(tileset->n, tileset->textureIds);
	free(tileset->textureIds);
	free(tileset);
}

int
find_fontcharacter(char ch){
	for(int i = 0; i < 64; i++)
		if (ch == font_characters[i][0])
			return font_characters[i][1];
	return -1;
}

void
hud_draw_string(int x, int y, int w, int h, char *str){
	char *p = str;
	int i = 0;
	while(*p != 0){
		int ind = find_fontcharacter(*p);
		if(ind != -1)
			hud_draw_tile_from_tileset(x + i * w, y, w, h, ind, font_tileset);
		i++;
		p++;
	}
}

tileset_t*
hud_load_tileset(char *path, int tw, int th, int nw, int nh, int border, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b, GLint filter){
	SDL_Surface *image = SDL_LoadBMP(path);
	if(image == NULL){
		char buff[100];
		snprintf(buff, 100, "Could not load image %s", path);
		util_fatalerror(__FILE__, __LINE__, buff);
	}

	tileset_t *tileset = malloc(sizeof(tileset_t));
	tileset->w = tw;
	tileset->h = th;
	tileset->n = nw * nh;
	tileset->textureIds = malloc(sizeof(GLuint) * tileset->n);
	glGenTextures(tileset->n, tileset->textureIds);

	Uint32 colorkey = SDL_MapRGB(image->format, ck_r, ck_g, ck_b);
	SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey); 
	for(int i = 0; i < nw; i++)
		for(int j = 0; j < nh; j++){
			SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, tw, th, 32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
						    0x000000FF, 
						    0x0000FF00, 
						    0x00FF0000, 
						    0xFF000000
#else
						    0xFF000000,
						    0x00FF0000, 
						    0x0000FF00, 
						    0x000000FF
#endif
						);				    
			SDL_Rect rect = { i * (tw + border) + border, j * (th + border) + border, tw, th } ;
			SDL_BlitSurface(image, &rect, tmp, NULL);
			
			glBindTexture(GL_TEXTURE_2D, tileset->textureIds[j * nw + i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

			SDL_FreeSurface(tmp);
		}

	SDL_FreeSurface(image);

	return tileset;
}

void
hud_draw_tile_from_tileset(int x, int y, int w, int h, int index, tileset_t *tileset){
	int width = w != -1 ? w : tileset->w;
	int height = h != -1 ? h : tileset->h;
	draw_quad_with_texture(x, y, width, height, tileset->textureIds[index]);
}

void
hud_draw_tile(int x, int y, int w, int h, tile_t *tile){
	int width = w != -1 ? w : tile->w;
	int height = h != -1 ? h : tile->h;
	draw_quad_with_texture(x, y, width, height, tile->textureId);
}

static void
draw_quad_with_texture(int x, int y, int width, int height, GLuint texId){
	GLint polymode;
	glGetIntegerv(GL_POLYGON_MODE, &polymode);
	if(polymode == GL_LINE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);	
	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, texId);
	glColor4f(1.0,1.0,1.0,1.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin(GL_QUADS);
	    glTexCoord2f(0, 1); glVertex3f(x, y, 0);
	    glTexCoord2f(1, 1); glVertex3f(x + width, y, 0);
	    glTexCoord2f(1, 0); glVertex3f(x + width, y + height, 0);
	    glTexCoord2f(0, 0); glVertex3f(x, y + height, 0);
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	if(polymode == GL_LINE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

tile_t*
hud_load_single_tile(char *path, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b, GLint filter){
	SDL_Surface *image = SDL_LoadBMP(path);
	if(image == NULL){
		char buff[100];
		snprintf(buff, 100, "Could not load image %s", path);
		util_fatalerror(__FILE__, __LINE__, buff);
	}

	tile_t *tile = malloc(sizeof(tile_t));

	SDL_Surface *tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, 32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                                    0x000000FF, 
                                    0x0000FF00, 
                                    0x00FF0000, 
                                    0xFF000000
#else
                                    0xFF000000,
                                    0x00FF0000, 
                                    0x0000FF00, 
                                    0x000000FF
#endif
				);				    
	Uint32 colorkey = SDL_MapRGB(image->format, ck_r, ck_g, ck_b);
	SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey); 
	SDL_BlitSurface(image, NULL, tmp, NULL);

	glGenTextures(1, &tile->textureId);
	glBindTexture(GL_TEXTURE_2D, tile->textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp->w, tmp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	tile->w = tmp->w;
	tile->h = tmp->h;

	SDL_FreeSurface(tmp);
	SDL_FreeSurface(image);

	return tile;
}

void
hud_unload_tile(tile_t *tile){
	glDeleteTextures(1, &tile->textureId);
	free(tile);
}

void
hud_draw_selection_cross(void){
	int x = (WINDOW_WIDTH - 33)/2;
	int y = (WINDOW_HEIGHT + 33)/2;

	hud_draw_tile(x, y, 33, 33, cross_tile);
}

static int
get_block_relative_to_chunk(int c_i, double v){
	long tmp;
	tmp = lround(v) % (2 * CHUNK_SIZE);
	return (int) lround((tmp + 0.5) / 2);	
}


static int 
shoot_ray(int *world_x, int *world_y, int *world_z){
	double v[3];

	v[0] = camera->eye[0];
	v[1] = camera->eye[1];
	v[2] = camera->eye[2];
	fflush(stdout);

	for(int i = 0; i < 200; i++){
		int c_ix = v[0] / (2 * CHUNK_SIZE);
		int c_iy = v[1] / (2 * CHUNK_SIZE);
		int c_iz = v[2] / (2 * CHUNK_SIZE);
		int b_x  = get_block_relative_to_chunk(c_ix, v[0]);
		int b_y  = get_block_relative_to_chunk(c_iy, v[1]);
		int b_z  = get_block_relative_to_chunk(c_iz, v[2]);
		if(b_x >= 0 && b_y >= 0 && b_z >= 0){

			printf("%d %d %d %d %d %d\n", c_ix, c_iy, c_iz, b_x, b_y, b_z);
			fflush(stdout);

			chunk_t *chunk = chunkmanager_get_chunk(c_ix, c_iy, c_iz);
			if(chunk != NULL && block_isactive(chunk->blocks[b_x][b_y][b_z])){
				*world_x = c_ix * (2 * CHUNK_SIZE) + 2 * b_x;
				*world_y = c_iy * (2 * CHUNK_SIZE) + 2 * b_y;
				*world_z = c_iz * (2 * CHUNK_SIZE) + 2 * b_z;
				printf("selected %d %d %d\n", *world_x, *world_y, *world_z);
				return 1;
			}
		}

		/* Advance */
		v[0] += 0.1 * camera->forward.x;
		v[1] += 0.1 * camera->forward.y;
		v[2] += 0.1 * camera->forward.z;
	}
	return 0;
}

void
hud_draw_selection_cube(void){
	int x, y, z;
	int r = shoot_ray(&x, &y, &z);
	if(r){
		char buff[200];
		memset(buff, 0, 200);
		snprintf(buff, 200, "x:%d y:%d z:%d", x/2, y/2, z/2);
		renderblock_outline(x, y, z);
		hud_draw_string(5, 520, 12, 16, buff);
	}
}

