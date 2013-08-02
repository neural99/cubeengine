#include <stdio.h>
#include <SDL/sdl.h>
#include <GL/gl.h>

#include "util.h"
#include "hud.h"

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
		printf("p=%c,ind=%d\n", *p, ind);
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
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WINDOW_WIDTH, 0.0, WINDOW_HEIGHT, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);	

	glBindTexture(GL_TEXTURE_2D, texId);
 
	glColor4f(1.0,1.0,1.0,1.0);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);
	    glTexCoord2f(0, 1); glVertex3f(x, y, 0);
	    glTexCoord2f(1, 1); glVertex3f(x + width, y, 0);
	    glTexCoord2f(1, 0); glVertex3f(x + width, y + height, 0);
	    glTexCoord2f(0, 0); glVertex3f(x, y + height, 0);
	glEnd();
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
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

