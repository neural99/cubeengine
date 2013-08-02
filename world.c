#include <SDL/sdl.h>
#include <GL/glee.h>

#include "util.h"
#include "world.h"

#define BLOCK_LENGTH 1.0f
#define BLOCK_HEIGHT 1.0f
#define BLOCK_WIDTH 1.0f

void
renderblock(int x, int y, int z){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x * 2 * BLOCK_LENGTH, y * 2 * BLOCK_HEIGHT, z * 2 * BLOCK_WIDTH);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);

		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
	glEnd();

	glPopMatrix();
}	

void
add_block_to_mesh(int x, int y, int z, mesh_t *mesh){
	float p1[3] = { x, y, z };		
	float p2[3] = { x + 1.0, y, z };
	float p3[3] = { x + 1.0, y + 1.0, z };
	float p4[3] = { x, y + 1.0, z };

	float p5[3] = { x, y, z + 1.0 };
	float p6[3] = { x + 1.0, y, z + 1.0 };
	float p7[3] = { x + 1.0, y + 1.0, z + 1.0 };
	float p8[3] = { x, y + 1.0, z + 1.0 };

	int i1 = mesh_add_vertex(mesh, p1);
	int i2 = mesh_add_vertex(mesh, p2);
	int i3 = mesh_add_vertex(mesh, p3);
	int i4 = mesh_add_vertex(mesh, p4);
	int i5 = mesh_add_vertex(mesh, p5);
	int i6 = mesh_add_vertex(mesh, p6);
	int i7 = mesh_add_vertex(mesh, p7);
	int i8 = mesh_add_vertex(mesh, p8);

	/* Front */
	mesh_add_trig(mesh, i1, i2, i3);
	mesh_add_trig(mesh, i3, i4, i1);

	/* Back */
	mesh_add_trig(mesh, i6, i5, i8);
	mesh_add_trig(mesh, i8, i7, i6);

	/* Top */
	mesh_add_trig(mesh, i3, i4, i8);
	mesh_add_trig(mesh, i8, i7, i3);

	/* Bottom */
	mesh_add_trig(mesh, i6, i2, i1);
	mesh_add_trig(mesh, i1, i5, i6);

	/* Left */
	mesh_add_trig(mesh, i1, i4, i8);
	mesh_add_trig(mesh, i8, i5, i1);

	/* Right */
	mesh_add_trig(mesh, i2, i3, i7);
	mesh_add_trig(mesh, i7, i6, i2);
}


void
chunk_build_mesh(chunk_t *chunk){
	for(int i = 0; i < CHUNK_SIZE; i++)
		for(int j = 0; j < CHUNK_SIZE; j++)
			for(int k = 0; k < CHUNK_SIZE; k++){
				printf("0x%X\n", block_isactive(chunk->blocks[i][j][k]));
				if(block_isactive(chunk->blocks[i][j][k])){
					printf("%X\n", chunk->blocks[i][j][k]);
					add_block_to_mesh(i, j, k, chunk->mesh);
				}
			}
}

void
chunk_rebuild(chunk_t *chunk){
	chunk_build_mesh(chunk);
	mesh_rebuild(chunk->mesh);
}

void
chuck_render(chunk_t *chunk){
	mesh_render(chunk->mesh);
}

mesh_t*
mesh_create(void){
	mesh_t *tmp = malloc(sizeof(mesh_t));
	tmp->vertex_list = util_list_create();
	tmp->n_verticies = 0;
	tmp->trig_list = util_list_create();
	tmp->n_trigs = 0;
	tmp->elementId = 0;
	tmp->vertexId = 0;
	return tmp;
}

void
mesh_free(mesh_t *m){
	util_list_free_data(m->vertex_list);
	util_list_free_data(m->trig_list);
	free(m);
}

int
mesh_add_vertex(mesh_t *m, float v[3]){
	float *vert = malloc(sizeof(float) * 3);
	memcpy(vert, v, 3 * sizeof(float));
	util_list_add(m->vertex_list, vert);
	return m->n_verticies++;
}

void
mesh_add_trig(mesh_t *m, GLuint ind1, GLuint ind2, GLuint ind3){
	GLuint *trig = malloc(sizeof(GLuint) * 3);

	/* Copy the indicies */
	GLuint *p = trig;
	*p++ = ind1;
	*p++ = ind2;
	*p++ = ind3;

	util_list_add(m->trig_list, trig);
	m->n_trigs++;
}

static void
copy_vertex_data(float *data, mesh_t *m){
	float *p = data;

	linked_list_elm_t *elm;
	elm = m->vertex_list->head;
	while(elm != NULL){
		float *vert = elm->data;
		*p++ = vert[0];
		*p++ = vert[1];
		*p++ = vert[2];

		elm = elm->next;
	}
}

static void
copy_index_data(GLuint *data, mesh_t *m){
	GLuint *p = data;

	linked_list_elm_t *elm;
	elm = m->trig_list->head;
	while(elm != NULL){
		GLuint *trig = elm->data;
		*p++ = trig[0];
		*p++ = trig[1];
		*p++ = trig[2];
		elm = elm->next;
	}
}

void
mesh_rebuild(mesh_t *m){
	/* Delete buffers if present */
	if(m->elementId != 0)
		glDeleteBuffers(1, &m->elementId);
	if(m->vertexId != 0)
		glDeleteBuffers(1, &m->vertexId);

	/* Fill vertex buffer */
	int size = m->n_verticies * 3 * sizeof(float);
	float *vertex_data = malloc(size);
	if(vertex_data == NULL)
		FATAL_ERROR("Out of memory");
	copy_vertex_data(vertex_data, m);
	glGenBuffers(1, &m->vertexId);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertexId);
	glBufferData(GL_ARRAY_BUFFER, size, vertex_data, GL_STATIC_DRAW);
	free(vertex_data);

	/* Fill indicies buffer */
	size = m->n_trigs * 3 * sizeof(GLuint);
	GLuint *index_data = malloc(size);
	if(index_data == NULL)
		FATAL_ERROR("Out of memory");
	copy_index_data(index_data, m);
	glGenBuffers(1, &m->elementId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index_data, GL_STATIC_DRAW);
	free(index_data);
}

void 
mesh_render(mesh_t *m){
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertexId);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementId);	

	glDrawElements(
			GL_TRIANGLES, 
			m->n_trigs,
			GL_UNSIGNED_INT,
			NULL
		      );

	glDisableClientState(GL_VERTEX_ARRAY);
}

void
chunk_render(chunk_t *c){
	mesh_render(c->mesh);
}

chunk_t*
chunk_create(void){
	chunk_t *tmp = malloc(sizeof(chunk_t));
	tmp->pos[0] = 0; tmp->pos[1] = 0; tmp->pos[2] = 0;
	for(int i = 0; i < CHUNK_SIZE; i++)
		for(int j = 0; j < CHUNK_SIZE; j++)
			for(int k = 0; k < CHUNK_SIZE; k++)
				tmp->blocks[i][j][k] = 0x80000000;
	tmp->mesh = mesh_create();

	chunk_rebuild(tmp);

	return tmp;
}

void
chunk_free(chunk_t *chunk){
	mesh_free(chunk->mesh);
	free(chunk);
}
