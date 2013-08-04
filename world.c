#include <SDL/sdl.h>
#include <GL/glee.h>
#include <string.h>

#include "util.h"
#include "world.h"
#include "camera.h"

#define BLOCK_LENGTH 500.0f
#define BLOCK_HEIGHT 500.0f
#define BLOCK_WIDTH 500.0f

typedef struct chunkmanager_s {
	linked_list_t *render_chunks;

	int active_blocks;
	int n_trigs;
} chunkmanager_t;

static chunkmanager_t *chunkmanager = NULL;

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
renderblock_with_textures(int x, int y, int z, GLuint texture){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);

	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	/* Front */
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glBegin(GL_QUADS);
		//glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0,1); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(0,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Back */
		//glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,1); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		/* Top */
		//glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0,1); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,1); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Bottom */
		//glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(0,0); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(1,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Left */
		//glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,1); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,0); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Right */
		//glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1,1); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord2f(0,1); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(0,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord2f(1,0); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

	glPopMatrix();
}	

void
add_block_to_mesh(int x, int y, int z, chunk_t *chunk){
	mesh_t *mesh = chunk->mesh;
	
	float p1[3] = { x - 1.0, y - 1.0, z + 1.0 };		
	float p2[3] = { x + 1.0, y - 1.0, z + 1.0};
	float p3[3] = { x + 1.0, y + 1.0, z + 1.0};
	float p4[3] = { x - 1.0, y + 1.0, z + 1.0};

	float p5[3] = { x + 1.0, y - 1.0, z - 1.0 };
	float p6[3] = { x - 1.0, y - 1.0, z - 1.0 };
	float p7[3] = { x - 1.0, y + 1.0, z - 1.0 };
	float p8[3] = { x + 1.0, y + 1.0, z - 1.0 };

	/* Green color */
	float c[3] = { 0.0f, 1.0f, 0.0f };

	/* Normals */
	float n1[3] = { 0.0f, 0.0f, 1.0f };
	float n2[3] = { 0.0f, 0.0f, -1.0f };
	float n3[3] = { 0.0f, 1.0f, 0.0f };
	float n4[3] = { 0.0f, -1.0f, 0.0f };
	float n5[3] = { -1.0f, 0.0f, 0.0f };
	float n6[3] = { 1.0f, 0.0f, 0.0f };

	int i1, i2, i3, i4, i5, i6, i7, i8;
	
	/* Front */
	int is_front_obscured = z + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[x][y][z+1]);
	if(!is_front_obscured){
		i1 = mesh_add_vertex(mesh, p1, c, n1);
		i2 = mesh_add_vertex(mesh, p2, c, n1);
		i3 = mesh_add_vertex(mesh, p3, c, n1);
		i4 = mesh_add_vertex(mesh, p4, c, n1);
		mesh_add_trig(mesh, i1, i2, i3);
		mesh_add_trig(mesh, i1, i3, i4);
	}

	/* Back */
	int is_back_obscured = z - 1 > 0 && block_isactive(chunk->blocks[x][y][z-1]);
	if(!is_back_obscured){
		i5 = mesh_add_vertex(mesh, p5, c, n2);
		i6 = mesh_add_vertex(mesh, p6, c, n2);
		i7 = mesh_add_vertex(mesh, p7, c, n2);
		i8 = mesh_add_vertex(mesh, p8, c, n2);
		mesh_add_trig(mesh, i5, i6, i7);
		mesh_add_trig(mesh, i5, i7, i8);
	}

	/* Top */
	int is_top_obscured = y + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[x][y+1][z]);
	if(!is_top_obscured){
		i3 = mesh_add_vertex(mesh, p3, c, n3);
		i4 = mesh_add_vertex(mesh, p4, c, n3);
		i7 = mesh_add_vertex(mesh, p7, c, n3);
		i8 = mesh_add_vertex(mesh, p8, c, n3);
		mesh_add_trig(mesh, i4, i3, i8);
		mesh_add_trig(mesh, i4, i8, i7);
	}

	/* Bottom */
	int is_bottom_obscured = y - 1 > 0 && block_isactive(chunk->blocks[x][y-1][z]);
	if(!is_bottom_obscured){
		i1 = mesh_add_vertex(mesh, p1, c, n4);
		i2 = mesh_add_vertex(mesh, p2, c, n4);
		i5 = mesh_add_vertex(mesh, p5, c, n4);
		i6 = mesh_add_vertex(mesh, p6, c, n4);
		mesh_add_trig(mesh, i6, i5, i2);
		mesh_add_trig(mesh, i6, i2, i1);
	}

	/* Left */
	int is_left_obscured = x - 1 > 0 && block_isactive(chunk->blocks[x-1][y][z]);
	if(!is_left_obscured){
		i1 = mesh_add_vertex(mesh, p1, c, n5);
		i4 = mesh_add_vertex(mesh, p4, c, n5);
		i6 = mesh_add_vertex(mesh, p6, c, n5);
		i7 = mesh_add_vertex(mesh, p7, c, n5);
		mesh_add_trig(mesh, i6, i1, i4);
		mesh_add_trig(mesh, i6, i4, i7);
	}

	/* Right */
	int is_right_obscured = x + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[x+1][y][z]);
	if(!is_right_obscured){
		i2 = mesh_add_vertex(mesh, p2, c, n6);
		i3 = mesh_add_vertex(mesh, p3, c, n6);
		i5 = mesh_add_vertex(mesh, p5, c, n6);
		i8 = mesh_add_vertex(mesh, p8, c, n6);
		mesh_add_trig(mesh, i2, i5, i8);
		mesh_add_trig(mesh, i2, i8, i3);
	}
}


void
chunk_build_mesh(chunk_t *chunk){
	for(int i = 0; i < CHUNK_SIZE; i++)
		for(int j = 0; j < CHUNK_SIZE; j++)
			for(int k = 0; k < CHUNK_SIZE; k++)
				if(block_isactive(chunk->blocks[i][j][k])){
					chunk->active_blocks++;
					add_block_to_mesh(i, j, k, chunk);
				}
			
}

void
chunk_rebuild(chunk_t *chunk){
	if(chunk->mesh->n_trigs != 0){
		mesh_free(chunk->mesh);
		chunk->mesh = mesh_create();
	}
	chunk->active_blocks = 0;
	chunk_build_mesh(chunk);
	mesh_rebuild(chunk->mesh);
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
mesh_free(void *p){
	mesh_t *m = p;
	if(m->elementId != 0)
		glDeleteBuffers(1, &m->elementId);
	if(m->vertexId != 0)
		glDeleteBuffers(1, &m->vertexId);
	util_list_free_data(m->vertex_list);
	util_list_free_data(m->trig_list);
	free(m);
}

int
mesh_add_vertex(mesh_t *m, float p[3], float c[3], float n[3]){
	float *vert = malloc(sizeof(float) * 9);
	memcpy(vert, p, 3 * sizeof(float));
	memcpy(vert + 3, c, 3 * sizeof(float));
	memcpy(vert + 6, n, 3 * sizeof(float));
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
		/* Position */
		*p++ = vert[0];
		*p++ = vert[1];
		*p++ = vert[2];

		/* Color */
		*p++ = vert[3];
		*p++ = vert[4];
		*p++ = vert[5];

		/* Normal */
		*p++ = vert[6];
		*p++ = vert[7];
		*p++ = vert[8];

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

static void
print_vertex_data(float *data, int size){
	float *p = data;
	int i = 0; 
	while(i < size){
		printf("%f %f %f\n", p[0], p[1], p[2]);

		i++;
		p+=3;
	}
}

static void
print_index_data(GLuint *data, int size){
	GLuint *p = data;
	int i = 0; 
	while(i < size){
		printf("%u %u %u\n", p[0], p[1], p[2]);

		i++;
		p+=3;
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
	int size = m->n_verticies * 9 * sizeof(float);
	float *vertex_data = malloc(size);
	if(vertex_data == NULL)
		FATAL_ERROR("Out of memory");
	copy_vertex_data(vertex_data, m);
	//print_vertex_data(vertex_data, m->n_verticies);
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
	//print_index_data(index_data, m->n_trigs);
	glGenBuffers(1, &m->elementId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index_data, GL_STATIC_DRAW);
	free(index_data);
}

void 
mesh_render(mesh_t *m){
	//printf("mesh render, vertexId=%d, elementId=%d\n", m->vertexId, m->elementId);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertexId);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 9 * sizeof(float), NULL);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 9 * sizeof(float), (void*) (3 * sizeof(float))); 

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 9 * sizeof(float), (void*) (6 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementId);	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDrawElements(
			GL_TRIANGLES, 
			m->n_trigs * 3,
			GL_UNSIGNED_INT,
			NULL
		      );

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void
chunk_render(chunk_t *c){
	//printf("chunk_render. pos =(%f %f %f)\n", c->pos[0], c->pos[1], c->pos[2]);
	glPushMatrix();
	glTranslated(c->pos[0], c->pos[1], c->pos[2]);
	mesh_render(c->mesh);
	glPopMatrix();
}

chunk_t*
chunk_create(void){
	chunk_t *tmp = malloc(sizeof(chunk_t));
	tmp->pos[0] = 0; tmp->pos[1] = 0; tmp->pos[2] = 0;
	memset(tmp->blocks, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Uint32));
	tmp->mesh = mesh_create();
	tmp->modified = 0;
	tmp->modified_list = util_list_create();
	tmp->active_blocks = 0;
	return tmp;
}

void
chunk_free(void *p){
	chunk_t *chunk = p;
	mesh_free(chunk->mesh);
	util_list_free_data(chunk->modified_list);
	free(chunk);
}

void 
chunk_add_modifid_block(chunk_t *c, int x, int y, int z){
	if(!c->modified) c->modified = 1;
	int *block_ind = malloc(sizeof(int) * 3);
	block_ind[0] = x;
	block_ind[1] = y;
	block_ind[2] = z;
	util_list_add(c->modified_list, block_ind);
}

int 
world_open(world_file_t *f){
	f->file = fopen(f->path, "r+");
	if(f->file == NULL){
		LOG_DEBUG("Could not open file %s", f->path);
		return -1;
	}
	Uint32 first;
	int r = fread(&first, sizeof(Uint32), 1, f->file);
	if(r != 1){
		LOG_DEBUG("Could not read magic number from file %s", f->path);
		return -1;
	}
	if(first != WORLD_FILE_MAGIC_NUMBER){
		LOG_DEBUG("File %s doesn't have correct magic number. Is it a world file?", f->path);
		return -1;
	}
	
	/* We probably have a world file. Read the world size */
	r = fread(&f->size, sizeof(Uint32), 3, f->file);
	if(r != 3){
		LOG_DEBUG("Could not read world size from file %s", f->path);
		return -1;
	}
	if(f->size[0] % CHUNK_SIZE != 0){
		LOG_DEBUG("Aborting. World length must be multiple of CHUNK_SIZE=%d", CHUNK_SIZE);
		return -1;
	}
	if((f->size[1] % CHUNK_SIZE) != 0){
		LOG_DEBUG("Aborting. World height must be multiple of CHUNK_SIZE=%d", CHUNK_SIZE);
		return -1;
	}
	if((f->size[2] % CHUNK_SIZE) != 0){
		LOG_DEBUG("Aborting. World width must be multiple of CHUNK_SIZE=%d", CHUNK_SIZE);
		return -1;
	}

	return 0;
}

int
world_read_chunk(world_file_t *f, int x, int y, int z, chunk_t *c){
	int chunk_linear_size = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
	int chunk_ind = z + y * f->size[2] / CHUNK_SIZE + x * f->size[2] / CHUNK_SIZE * f->size[1] / CHUNK_SIZE;
	int byte_offset = (4 + chunk_ind * chunk_linear_size) * sizeof(Uint32);
	if(fseek(f->file, byte_offset, SEEK_SET) < 0){
		LOG_DEBUG("Couldnt seek to read chunk (%d %d %d)", x, y, z);
		return -1;
	}

	int r = fread(c->blocks, sizeof(Uint32), chunk_linear_size, f->file); 	
	if(r != chunk_linear_size){
		LOG_DEBUG("Could not read chunk (%d %d %d)", x, y, z);
		return -1;
	}
	int active = 0;
	for(int i = 0; i < CHUNK_SIZE; i++)
		for(int j = 0; j < CHUNK_SIZE; j++)
			for(int k = 0; k < CHUNK_SIZE; k++)
				if(block_isactive(c->blocks[i][j][k]))
					active++;

	printf("%d\n", active);
	c->pos[0] = x * (CHUNK_SIZE + 1);
	c->pos[1] = y * (CHUNK_SIZE + 1);
	c->pos[2] = z * (CHUNK_SIZE + 1);

	return 0;
}

void
chunkmanager_init(world_file_t *world){
	chunkmanager = malloc(sizeof(chunkmanager_t));
	chunkmanager->render_chunks = util_list_create();
	chunkmanager->active_blocks = 0;
	chunkmanager->n_trigs = 0;
	
	for(int i = 0; i < (int)world->size[0] / CHUNK_SIZE; i++)
		for(int j = 0; j < (int)world->size[1] / CHUNK_SIZE; j++)
			for(int k = 0; k < (int)world->size[2] / CHUNK_SIZE; k++){
				chunk_t *c = chunk_create();
				world_read_chunk(world, i, j, k, c);
				util_list_add(chunkmanager->render_chunks, c);
			}
}

void
chunkmanager_rebuild(void){
	linked_list_elm_t *elm;

	elm = chunkmanager->render_chunks->head;
	while(elm != NULL){
		chunk_t *c = elm->data;
		chunk_rebuild(c);
		chunkmanager->active_blocks += c->active_blocks;
		chunkmanager->n_trigs += c->mesh->n_trigs;
		elm = elm->next;
	}
}	

void
chunkmanager_render_world(void){
	linked_list_elm_t *elm;

	elm = chunkmanager->render_chunks->head;
	while(elm != NULL){
		chunk_t *c = elm->data;
		chunk_render(c);
		elm = elm->next;
	}
}

void
chunkmanager_free(void){
	util_list_free_custom(chunkmanager->render_chunks, chunk_free);
	free(chunkmanager);
}

int
chunkmanager_nchunks(void){
	return util_list_size(chunkmanager->render_chunks);
}

int 
chunkmanager_activeblocks(void){
	return chunkmanager->active_blocks;
}

int 
chunkmanager_ntrigs(void){
	return chunkmanager->n_trigs;
}

static GLuint
load_cubemap(char *dir){
	GLuint textureId;

	char *names[6] = {"front.bmp", "back.bmp", "top.bmp", "bottom.bmp", "left.bmp", "right.bmp"};
	SDL_Surface *teximgs[6];

	for(int i = 0; i < 6; i++){
		char path[400];
		strcpy(path, dir);
		strcat(path, "/");
		strcat(path, names[i]);

		SDL_Surface *image = SDL_LoadBMP(path);
		if(image == NULL)
			FATAL_ERROR("Could not load image %s", path);

		teximgs[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, 32, 
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
		
		SDL_BlitSurface(image, NULL, teximgs[i], NULL);
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, teximgs[0]->w, teximgs[0]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[0]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, teximgs[1]->w, teximgs[1]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[1]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, teximgs[2]->w, teximgs[2]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[2]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, teximgs[3]->w, teximgs[3]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[3]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, teximgs[4]->w, teximgs[4]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[4]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, teximgs[5]->w, teximgs[5]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[5]->pixels);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for(int i = 0; i < 6; i++) SDL_FreeSurface(teximgs[i]);
	SDL_FreeSurface(image);

	return textureId;
}

skybox_t*
skybox_create(char *texture_dir){
	skybox_t *sb = malloc(sizeof(skybox_t));
	sb->textureId = load_cubemap("skybox");

	return sb;
}

void
skybox_render(skybox_t *sb){
	renderblock_with_textures(camera->eye[0], camera->eye[1], camera->eye[2], sb->textureId);
}
