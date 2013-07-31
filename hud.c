#include <stdio.h>
#include <SDL/sdl.h>
#include <GL/gl.h>

#include "util.h"
#include "hud.h"

static tileset_t *font_tileset;
static int font_characters[][2] = { { '0', 16 },
				   { '1', 17 },
				   { '2', 18 },
				   { '3', 19 },
				   { '4', 20 },
				   { '5', 21 },
				   { '6', 22 },
				   { '7', 23 },
				   { '8', 24 },
				   { '9', 25 }};

void 
hud_init(void){
	font_tileset = hud_load_tileset("font.bmp", 6, 8, 16, 6, 1, 0xFF, 0x00, 0xFF);
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
	for(int i = 0; i < 10; i++)
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
hud_load_tileset(char *path, int tw, int th, int nw, int nh, int border, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b){
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
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			SDL_FreeSurface(tmp);
		}

	SDL_FreeSurface(image);

	return tileset;
}

void
hud_draw_tile_from_tileset(int x, int y, int w, int h, int index, tileset_t *tileset){
	int width = w != -1 ? w : tileset->w;
	int height = h != -1 ? h : tileset->h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800, 800, 0.0, -1.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);	

	glBindTexture(GL_TEXTURE_2D, tileset->textureIds[index]);
 
	glColor4f(1.0,1.0,1.0,1.0);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	    glTexCoord2f(0, 0); glVertex3f(x, y, 0);
	    glTexCoord2f(1, 0); glVertex3f(x + width, y, 0);
	    glTexCoord2f(1, 1); glVertex3f(x + width, y + height, 0);
	    glTexCoord2f(0, 1); glVertex3f(x, y + height, 0);
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
}

void
hud_draw_tile(int x, int y, int w, int h, tile_t *tile){
	int width = w != -1 ? w : tile->w;
	int height = h != -1 ? h : tile->h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 800, 800, 0.0, -1.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_DEPTH_BUFFER_BIT);	

	glBindTexture(GL_TEXTURE_2D, tile->textureId);
 
	glColor4f(1.0,1.0,1.0,1.0);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	    glTexCoord2f(0, 0); glVertex3f(x, y, 0);
	    glTexCoord2f(1, 0); glVertex3f(x + width, y, 0);
	    glTexCoord2f(1, 1); glVertex3f(x + width, y + height, 0);
	    glTexCoord2f(0, 1); glVertex3f(x, y + height, 0);
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
}

tile_t*
hud_load_single_tile(char *path, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b){
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

