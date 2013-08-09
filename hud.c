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

#include <stdio.h>
#include <math.h>
#include <SDL/sdl.h>
#include <GL/glee.h>
#include <GL/glu.h>

#include "util.h"
#include "hud.h"
#include "world.h"
#include "camera.h"
#include "startup.h"

int hud_selected_block[3];
chunk_t *hud_selected_chunk;

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
				   { ' ', 0  },
				   { '>', 30 },
				   { '_', 63 },
				   { '(', 91 },
				   { '|', 92 },
				   { ')', 93 },
				   { '.', 13 },
				   { ',', 12 },
				   { '-', 14 }};

static void draw_quad_with_texture(int x, int y, int width, int height, float alpha, GLuint texId);

void 
hud_init(void){
	font_tileset = hud_load_tileset("font.bmp", 6, 8, 16, 6, 1, 0xFF, 0x00, 0x00, GL_NEAREST);
	cross_tile = hud_load_single_tile("cross.bmp", 0xff, 0x00, 0xff, GL_NEAREST);
	hud_selected_block[0] = -1;
	hud_selected_block[1] = -1;
	hud_selected_block[2] = -1;
}
STARTUP_PROC(hud, 3, hud_init)

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
	for(int i = 0; i < 72; i++)
		if (ch == font_characters[i][0])
			return font_characters[i][1];
	return -1;
}

void
hud_draw_string_with_alpha(int x, int y, int w, int h, float alpha, char *str){
	char *p = str;
	int i = 0;
	while(*p != 0){
		int ind = find_fontcharacter(*p);
		if(ind != -1)
			hud_draw_tile_from_tileset_with_alpha(x + i * w, y, w, h, ind, alpha, font_tileset);
		else
			hud_draw_tile_from_tileset_with_alpha(x + i * w, y, w, h, 31, alpha, font_tileset);
		i++;
		p++;
	}
}

void
hud_draw_string(int x, int y, int w, int h, char *str){
	hud_draw_string_with_alpha(x, y, w, h, 1.0, str);
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
hud_draw_tile_from_tileset_with_alpha(int x, int y, int w, int h, int index, float alpha, tileset_t *tileset){
	int width = w != -1 ? w : tileset->w;
	int height = h != -1 ? h : tileset->h;
	draw_quad_with_texture(x, y, width, height, alpha, tileset->textureIds[index]);
}

void
hud_draw_tile_from_tileset(int x, int y, int w, int h, int index, tileset_t *tileset){
	hud_draw_tile_from_tileset_with_alpha(x, y, w, h, index, 1.0, tileset);
}

void
hud_draw_tile_with_alpha(int x, int y, int w, int h, float alpha, tile_t *tile){
	int width = w != -1 ? w : tile->w;
	int height = h != -1 ? h : tile->h;
	draw_quad_with_texture(x, y, width, height, alpha, tile->textureId);
}

void
hud_draw_tile(int x, int y, int w, int h, tile_t *tile){
	hud_draw_tile_with_alpha(x, y, w, h, 1.0, tile);
}

static void
draw_quad_with_texture(int x, int y, int width, int height, float alpha, GLuint texId){
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
	glColor4f(1.0,1.0,1.0,alpha);
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
get_block_relative_to_chunk(double v){
	long tmp;
	tmp = lround(v) % (2 * CHUNK_SIZE);
	int t = (int) lround((tmp + 0.5) / 2);	
	if(t >= CHUNK_SIZE)
		t = CHUNK_SIZE - 1;
	return t;
}

static double 
len_to_eye(double v[3]){
	double tmp[3];
	tmp[0] = camera->eye[0] - v[0];
	tmp[1] = camera->eye[1] - v[1];
	tmp[2] = camera->eye[2] - v[2];
	return length(tmp);
}


static int 
shoot_ray(int *out_x, int *out_y, int *out_z, chunk_t **out_c){
	double v[3];

	v[0] = camera->eye[0];
	v[1] = camera->eye[1];
	v[2] = camera->eye[2];

	while(len_to_eye(v) < 15){
		int c_ix = v[0] / (2 * CHUNK_SIZE);
		int c_iy = v[1] / (2 * CHUNK_SIZE);
		int c_iz = v[2] / (2 * CHUNK_SIZE);
		int b_x  = get_block_relative_to_chunk(v[0]);
		int b_y  = get_block_relative_to_chunk(v[1]);
		int b_z  = get_block_relative_to_chunk(v[2]);
		if(b_x >= 0 && b_y >= 0 && b_z >= 0){
			chunk_t *chunk = chunkmanager_get_chunk(c_ix, c_iy, c_iz);
			if(chunk != NULL && block_isactive(chunk->blocks[b_x][b_y][b_z])){
				*out_x = c_ix * CHUNK_SIZE + b_x;
				*out_y = c_iy * CHUNK_SIZE + b_y;
				*out_z = c_iz * CHUNK_SIZE + b_z;
				*out_c = chunk;
				//printf("selected %d %d %d\n", *world_x, *world_y, *world_z);
				return 1;
			}
		}

		/* Advance */
		v[0] += 0.5 * camera->forward.x;
		v[1] += 0.5 * camera->forward.y;
		v[2] += 0.5 * camera->forward.z;
	}
	return 0;
}

void
hud_draw_selection_cube(void){
	int x, y, z;
	chunk_t *c;
	int r = shoot_ray(&x, &y, &z, &c);
	if(r){
		/* Store in global variables */
		hud_selected_chunk = c;
		hud_selected_block[0] = x;
		hud_selected_block[1] = y;
		hud_selected_block[2] = z;

		/* Draw debug information */
		char buff[200];
		memset(buff, 0, 200);
		snprintf(buff, 200, "x:%d y:%d z:%d", x, y, z);
		renderblock_outline(2*x, 2*y, 2*z);
		hud_draw_string(5, 520, 12, 16, buff);
	}
}

