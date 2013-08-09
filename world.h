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

#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL/sdl.h>
#include <GL/glee.h>

#include "util.h"

typedef Uint32 block_t;

#define block_isactive(block) ((block & 0x80000000) >> 31)  
#define block_type(block) ( block & 0x70000000 )

#define CHUNK_SIZE 16
#define MAX_ACTIVE_BLOCKS ( CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE )

typedef struct mesh_s {
	linked_list_t *vertex_list;
	int n_verticies;
	linked_list_t *trig_list;
	int n_trigs;
	GLuint elementId, vertexId;
} mesh_t;

mesh_t* mesh_create(void);
int mesh_add_vertex(mesh_t *m, float p[3], float c[3], float n[3], float t[2]);
void mesh_add_trig(mesh_t *m, GLuint ind1, GLuint ind2, GLuint ind3);
void mesh_rebuild(mesh_t *m);
void mesh_render(mesh_t *m);
void mesh_free(void *p);

typedef struct chunk_s {
	/* Position of origo in world coordinates */
	double pos[3]; 
	/* Chunk position in chunk coordinates */
	int ix, iy, iz;
	block_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	mesh_t *mesh;
	int modified;
	linked_list_t *modified_list;					
	int active_blocks;
} chunk_t;

chunk_t* chunk_create(void);
void chunk_rebuild(chunk_t *chunk);
void chunk_render(chunk_t *chunk);
void chunk_free(void *p);
void chunk_add_modified_block(chunk_t *chunk, int ix, int iy, int iz);
/* Remove the block with world block coordinates (w_x, w_y, w_z) */
void chunk_remove_block(chunk_t *c, int w_x, int w_y, int w_z);
void chunk_add_block(chunk_t *c, Uint32 block_type, int w_x, int w_y, int w_z);

#define WORLD_FILE_MAGIC_NUMBER 274263364

typedef struct world_file_s {
	/* Input */
	char *path;
	/* Output */
	Uint32 size[3]; /* Size in blocks */
	/* Internal */
	FILE *file;
} world_file_t;

int world_open(world_file_t *f);
int world_read_chunk(world_file_t *f, int x, int y, int z, chunk_t *chunk);
/* Push the modified part of chunk to disk */
void world_update_chunk(world_file_t *f, chunk_t *chunk);
/* (Re-)write the entire world file to disk. */
void world_write_file(world_file_t *f);

void chunkmanager_init(world_file_t *f);
void chunkmanager_rebuild(void);
void chunkmanager_render_world(void);
void chunkmanager_free(void);
int chunkmanager_nchunks(void);
int chunkmanager_activeblocks(void);
int chunkmanager_ntrigs(void);
chunk_t* chunkmanager_get_chunk(int, int, int);
void chunkmanager_update(void);

typedef struct skybox_s {
	GLuint textureId;
} skybox_t;

skybox_t *skybox_create(void);
void skybox_render(skybox_t *sb);

/* Block texture set */
void textureset_init(void);
GLuint textureset_current_atlas(void);
void textureset_free(void);
GLfloat* textureset_texcoords(Uint32 block_type, int face, int vert);
void textureset_bind(void);
void textureset_unbind(void);

/* Misc utility */
void renderblock(int x, int y, int z);
void renderblock_with_textures(int x, int y, int z, GLuint cubemap);
void renderblock_outline(int x, int y, int z);

#endif
