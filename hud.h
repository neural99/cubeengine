#ifndef __HUD_H__
#define __HUD_H__

#include <GL/glee.h>
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
void hud_draw_tile_from_tileset(int x, int y, int w, int h, int index, tileset_t *set);
void hud_draw_string(int x, int y, int w, int h, char *str);

void hud_cleanup(void);

/* Selection tool */
void hud_draw_selection_cross(void);
void hud_draw_selection_cube(void);

extern int hud_selected_block[3];
extern chunk_t *hud_selected_chunk;

#endif
