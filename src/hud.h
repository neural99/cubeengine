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

#ifndef __HUD_H__
#define __HUD_H__

#include <GLee.h>
#include "world.h"

typedef struct tile_s {
	int w;
	int h;
	GLuint textureId;
} tile_t;

typedef struct tileset_s {
	int w;
	int h;
	/* Array of texture Ids */
	GLuint *textureIds;
	/* Number of tiles in tileset */
	int n;
} tileset_t;

void hud_init(void);

tileset_t* hud_load_tileset(char *path, int tw, int th, int nw, int nh, int border, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b, GLint filter);
tile_t* hud_load_single_tile(char *path, Uint8 ck_r, Uint8 ck_g, Uint8 ck_b, GLint filter);
void hud_unload_tile(tile_t *tile);
void hud_unload_tileset(tileset_t *set);
void hud_draw_tile(int x, int y, int w, int h, tile_t *tile);
void hud_draw_tile_with_alpha(int x, int y, int w, int h, float alpha, tile_t *tile);
void hud_draw_tile_from_tileset(int x, int y, int w, int h, int index, tileset_t *set);
void hud_draw_tile_from_tileset_with_alpha(int x, int y, int w, int h, int index, float alpha, tileset_t *set);
void hud_draw_string(int x, int y, int w, int h, char *str);
void hud_draw_string_with_alpha(int x, int y, int w, int h, float alpha, char *str);

void hud_cleanup(void);

/* Selection tool */
void hud_draw_selection_cross(void);
void hud_draw_selection_cube(void);

extern int hud_selected_block[3];
extern chunk_t *hud_selected_chunk;

#endif
