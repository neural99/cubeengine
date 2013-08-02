#ifndef __WORLD_H__
#define __WORLD_H__

#include <SDL/sdl.h>

#include "util.h"

typedef Uint32 block_t;

#define block_isactive(block) ((block & 0x80000000) >> 31)  
#define block_type(block) ( block & 0x70000000 )

#define CHUNK_SIZE 16

typedef struct mesh_s {
	linked_list_t *vertex_list;
	int n_verticies;
	linked_list_t *trig_list;
	int n_trigs;
	GLuint elementId, vertexId;
} mesh_t;

mesh_t* mesh_create(void);
int mesh_add_vertex(mesh_t *m, float v[3]);
void mesh_add_trig(mesh_t *m, GLuint ind1, GLuint ind2, GLuint ind3);
void mesh_rebuild(mesh_t *m);
void mesh_render(mesh_t *m);
void mesh_free(mesh_t *m);

typedef struct chunk_s {
	/* Position of origo */
	double pos[3]; 
	block_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	mesh_t *mesh;
} chunk_t;

chunk_t* chunk_create(void);
void chunk_rebuild(chunk_t *chunk);
void chunk_render(chunk_t *chunk);
void chunk_free(chunk_t *chunk);

void renderblock(int x, int y, int z);

#endif
