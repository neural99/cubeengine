#include <string.h>
#include <math.h>
#include <SDL/sdl.h>
#include <GL/glee.h>

#include "util.h"
#include "world.h"
#include "camera.h"
#include "startup.h"

#define BLOCK_LENGTH 500.0f
#define BLOCK_HEIGHT 500.0f
#define BLOCK_WIDTH 500.0f

typedef struct chunkmanager_s {
	linked_list_t *render_chunks;
	linked_list_t *loaded_chunks;

	int active_blocks;
	int n_trigs;
} chunkmanager_t;

static chunkmanager_t *chunkmanager = NULL;

/* TODO: Read this from settings file */
static char *textureset_atlas_path = "block_textures.bmp";
static int textureset_size = 32;
static char *textureset_mapping_path = "block_texture_mappings.txt";

typedef struct block_type_textures_s {
	Uint32 block_type;
	Uint32 texture_indicies[6];
} block_type_textures_t;

typedef struct textureset_s {
	GLuint textureId;
	int atlas_w;
	int atlas_h;
	int n_subtextures;
	int n_w, n_h;
	linked_list_t *block_type_mappings;
} textureset_t;

static textureset_t textureset_current;

static block_type_textures_t*
get_block_type_textures(Uint32 block_type){
	linked_list_elm_t *elm;

	elm = textureset_current.block_type_mappings->head;
	while(elm != NULL){
		block_type_textures_t *btt = elm->data;	
		if(btt->block_type == block_type)
			return btt;
		elm = elm->next;
	}
	/* Nothing found */
	return NULL;
}

static void
load_block_type_mappings(char *path){
	FILE *f = fopen(path, "r");
	if(f == NULL)
		FATAL_ERROR("Could not load textureset mapping file %s", path);

	int line = 0;
	char buff[1024];
	Uint32 block_type, front, back, top, bottom, left, right;
	while(!feof(f)){
		char *r = fgets(buff, 1024, f);
		if(r == NULL)
			break;

		int items = sscanf(buff, "%d %d %d %d %d %d %d", (int*)&block_type, (int*)&front, (int*)&back,
								(int*)&top, (int*)&bottom, (int*)&left, (int*)&right);
		if(items =! 7)
			FATAL_ERROR("Error parsing block type mapping file %s. Error at line %d", path, line);
		
		/* Add to list */
		block_type_textures_t *block_tex = malloc(sizeof(block_type_textures_t));
		block_tex->block_type = block_type;
		block_tex->texture_indicies[0] = front;
		block_tex->texture_indicies[1] = back;
		block_tex->texture_indicies[2] = top;
		block_tex->texture_indicies[3] = bottom;
		block_tex->texture_indicies[4] = left; 
		block_tex->texture_indicies[5] = right; 

		util_list_add(textureset_current.block_type_mappings, block_tex);

		line++;
	}
}

static void
textureset_load_texture_atlas(char *path, int size){
	GLuint textureId;

	SDL_Surface *image = SDL_LoadBMP(path);
	if(image == NULL)
		FATAL_ERROR("Could not open texture set %s", path);

	if(image->w % size != 0)
		FATAL_ERROR("The width of texture set %s  must be a multiple of %d", path, size);
	if(image->h % size != 0)
		FATAL_ERROR("The height of texture set %s  must be a multiple of %d", path, size);

	image = SDL_DisplayFormat(image);
	if(image == NULL)
		FATAL_ERROR("SDL_DisplayFormat failed. Out of memeroy?");

	textureset_current.atlas_w = image->w;
	textureset_current.atlas_h = image->h;

	int n_w = image->w / size;
	int n_h = image->h / size;
	textureset_current.n_w = n_w;
	textureset_current.n_h = n_h;
	textureset_current.n_subtextures = n_w * n_h;

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
	SDL_BlitSurface(image, NULL, tmp, NULL);

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp->w, tmp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_FreeSurface(image);
	SDL_FreeSurface(tmp);

	textureset_current.textureId = textureId;
}


GLuint
textureset_current_atlas(void){
	return textureset_current.textureId;
}

GLfloat*
textureset_texcoords(Uint32 block_type, int face, int vert){
	block_type_textures_t *btt = get_block_type_textures(block_type);
	if(btt == NULL)
		FATAL_ERROR("Could not find block type texture mappings for block type = %d. Aborting", block_type);
	int tex_ind = btt->texture_indicies[face];
	/* Convert linear index to (x, y) sub texture index */
	int n_y = tex_ind / textureset_current.n_w;
	int n_x = tex_ind % textureset_current.n_w;
	/* Pixel coordinates */
	int y = n_y * textureset_size;
	int x = n_x * textureset_size;

	//printf("%d\n", textureset_current.atlas_w);

	GLfloat *uv = malloc(sizeof(GLfloat) * 2);

	/* Counting CCW from (0,0) */
	if(vert == 0){
		uv[0] =  x / (float)textureset_current.atlas_w;
	        uv[1] =	 y / (float)textureset_current.atlas_h; 
	}else if(vert == 1){
		uv[0] =  (x + textureset_size) / (float)textureset_current.atlas_w;
	        uv[1] =	 y / (float)textureset_current.atlas_h; 
	}else if(vert == 2){
		uv[0] =  (x + textureset_size) / (float)textureset_current.atlas_w;
	        uv[1] =	 (y + textureset_size) / (float)textureset_current.atlas_h; 
	}else if(vert == 3){
		uv[0] =  x / (float)textureset_current.atlas_w;
	        uv[1] =	 (y + textureset_size) / (float)textureset_current.atlas_h; 
	}else{
		FATAL_ERROR("Invalid input to function. Should not happend.");
	}
	
	//printf("u=%f,v=%f\n", uv[0],uv[1]);

	return uv;
}

void
textureset_init(void){
	textureset_current.block_type_mappings = util_list_create();
	load_block_type_mappings(textureset_mapping_path);
	textureset_load_texture_atlas(textureset_atlas_path, textureset_size); 
}
STARTUP_PROC(textureset, 4, textureset_init)

void 
textureset_free(void){
	util_list_free_data(textureset_current.block_type_mappings);
	glDeleteTextures(1, &textureset_current.textureId);
	textureset_current.textureId = 0;
	textureset_current.n_subtextures = 0;
}

void
textureset_bind(void){
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureset_current.textureId);
}

void
textureset_unbind(void){
	glDisable(GL_TEXTURE_2D);	
}

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
renderblock_outline(int x, int y, int z){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f( 1.1f, -1.1f, -1.1f);
		glVertex3f(-1.1f, -1.1f, -1.1f);
		glVertex3f(-1.1f,  1.1f, -1.1f);
		glVertex3f( 1.1f,  1.1f, -1.1f);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-1.1f, -1.1f,  1.1f);
		glVertex3f( 1.1f, -1.1f,  1.1f);
		glVertex3f( 1.1f,  1.1f,  1.1f);
		glVertex3f(-1.1f,  1.1f,  1.1f);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f( 1.1f, -1.1f,  1.1f);
		glVertex3f( 1.1f, -1.1f, -1.1f);
		glVertex3f( 1.1f,  1.1f, -1.1f);
		glVertex3f( 1.1f,  1.1f,  1.1f);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-1.1f, -1.1f, -1.1f);
		glVertex3f(-1.1f, -1.1f,  1.1f);
		glVertex3f(-1.1f,  1.1f,  1.1f);
		glVertex3f(-1.1f,  1.1f, -1.1f);

		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-1.1f, -1.1f, -1.1f);
		glVertex3f( 1.1f, -1.1f, -1.1f);
		glVertex3f( 1.1f, -1.1f,  1.1f);
		glVertex3f(-1.1f, -1.1f,  1.1f);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f( 1.1f,  1.1f, -1.1f);
		glVertex3f(-1.1f,  1.1f, -1.1f);
		glVertex3f(-1.1f,  1.1f,  1.1f);
		glVertex3f( 1.1f,  1.1f,  1.1f);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix();
}	

void
renderblock_with_textures(int x, int y, int z, GLuint texture){
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);

	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_CULL_FACE);

	float N = 1.0f / sqrt(3.0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glBegin(GL_QUADS);
		/* Front */
		//glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord3f(-N,-N,N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,-N,N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,N,N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(-N,N,N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Back */
		//glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord3f(N,-N,-N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,-N,-N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,N,-N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(N,N,-N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		/* Top */
		//glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord3f(N,N,-N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,N,-N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,N,N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,N,N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Bottom */
		//glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord3f(-N,-N,-N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,-N,N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,-N,N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,-N,-N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		/* Left */
		//glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord3f(N,-N,N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(N,-N,-N); glVertex3f( BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(N,N,-N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(N,N,N); glVertex3f( BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		/* Right */
		//glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord3f(-N,-N,-N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT, -BLOCK_WIDTH);
		glTexCoord3f(-N,-N,N); glVertex3f(-BLOCK_LENGTH, -BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(-N,N,N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT,  BLOCK_WIDTH);
		glTexCoord3f(-N,N,-N); glVertex3f(-BLOCK_LENGTH,  BLOCK_HEIGHT, -BLOCK_WIDTH);
	glEnd();
	glDisable(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

	glPopMatrix();
}	

void
add_block_to_mesh(int i, int j, int k, chunk_t *chunk){
	mesh_t *mesh = chunk->mesh;
	int x = 2 * i; int y = 2 * j; int z = 2 * k;
	
	float p1[3] = { x - 1.0, y - 1.0, z + 1.0 };		
	float p2[3] = { x + 1.0, y - 1.0, z + 1.0};
	float p3[3] = { x + 1.0, y + 1.0, z + 1.0};
	float p4[3] = { x - 1.0, y + 1.0, z + 1.0};

	float p5[3] = { x + 1.0, y - 1.0, z - 1.0 };
	float p6[3] = { x - 1.0, y - 1.0, z - 1.0 };
	float p7[3] = { x - 1.0, y + 1.0, z - 1.0 };
	float p8[3] = { x + 1.0, y + 1.0, z - 1.0 };

	/* Color. To be used to control lightning */
	float c[3] = { 1.0f, 1.0f, 1.0f };

	/* Normals */
	float n1[3] = { 0.0f, 0.0f, 1.0f };
	float n2[3] = { 0.0f, 0.0f, -1.0f };
	float n3[3] = { 0.0f, 1.0f, 0.0f };
	float n4[3] = { 0.0f, -1.0f, 0.0f };
	float n5[3] = { -1.0f, 0.0f, 0.0f };
	float n6[3] = { 1.0f, 0.0f, 0.0f };

	int i1, i2, i3, i4, i5, i6, i7, i8;
	GLfloat* uv;
	
	/* Front */
	int is_front_obscured = k + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[i][j][k+1]);
	if(!is_front_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 0, 0);
		i1 = mesh_add_vertex(mesh, p1, c, n1, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 0, 1);
		i2 = mesh_add_vertex(mesh, p2, c, n1, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 0, 2);
		i3 = mesh_add_vertex(mesh, p3, c, n1, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 0, 3);
		i4 = mesh_add_vertex(mesh, p4, c, n1, uv);
		free(uv);
		mesh_add_trig(mesh, i1, i2, i3);
		mesh_add_trig(mesh, i1, i3, i4);
	}

	/* Back */
	int is_back_obscured = k - 1 >= 0 && block_isactive(chunk->blocks[i][j][k-1]);
	if(!is_back_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 1, 0);
		i5 = mesh_add_vertex(mesh, p5, c, n2, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 1, 1);
		i6 = mesh_add_vertex(mesh, p6, c, n2, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 1, 2);
		i7 = mesh_add_vertex(mesh, p7, c, n2, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 1, 3);
		i8 = mesh_add_vertex(mesh, p8, c, n2, uv);
		free(uv);
		mesh_add_trig(mesh, i5, i6, i7);
		mesh_add_trig(mesh, i5, i7, i8);
	}

	/* Top */
	int is_top_obscured = j + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[i][j+1][k]);
	if(!is_top_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 2, 0);
		i3 = mesh_add_vertex(mesh, p3, c, n3, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 2, 1);
		i4 = mesh_add_vertex(mesh, p4, c, n3, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 2, 2);
		i7 = mesh_add_vertex(mesh, p7, c, n3, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 2, 3);
		i8 = mesh_add_vertex(mesh, p8, c, n3, uv);
		free(uv);
		mesh_add_trig(mesh, i4, i3, i8);
		mesh_add_trig(mesh, i4, i8, i7);
	}

	/* Bottom */
	int is_bottom_obscured = j - 1 >= 0 && block_isactive(chunk->blocks[i][j-1][k]);
	if(!is_bottom_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 3, 0);
		i1 = mesh_add_vertex(mesh, p1, c, n4, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 3, 1);
		i2 = mesh_add_vertex(mesh, p2, c, n4, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 3, 2);
		i5 = mesh_add_vertex(mesh, p5, c, n4, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 3, 3);
		i6 = mesh_add_vertex(mesh, p6, c, n4, uv);
		free(uv);
		mesh_add_trig(mesh, i6, i5, i2);
		mesh_add_trig(mesh, i6, i2, i1);
	}

	/* Left */
	int is_left_obscured = i - 1 >= 0 && block_isactive(chunk->blocks[i-1][j][k]);
	if(!is_left_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 4, 0);
		i1 = mesh_add_vertex(mesh, p1, c, n5, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 4, 1);
		i4 = mesh_add_vertex(mesh, p4, c, n5, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 4, 2);
		i6 = mesh_add_vertex(mesh, p6, c, n5, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 4, 3);
		i7 = mesh_add_vertex(mesh, p7, c, n5, uv);
		free(uv);
		mesh_add_trig(mesh, i6, i1, i4);
		mesh_add_trig(mesh, i6, i4, i7);
	}

	/* Right */
	int is_right_obscured = i + 1 < CHUNK_SIZE && block_isactive(chunk->blocks[i+1][j][k]);
	if(!is_right_obscured){
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 5, 0);
		i2 = mesh_add_vertex(mesh, p2, c, n6, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 5, 1);
		i3 = mesh_add_vertex(mesh, p3, c, n6, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 5, 2);
		i5 = mesh_add_vertex(mesh, p5, c, n6, uv);
		free(uv);
		uv = textureset_texcoords(block_type(chunk->blocks[i][j][k]), 5, 3);
		i8 = mesh_add_vertex(mesh, p8, c, n6, uv);
		free(uv);
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
mesh_add_vertex(mesh_t *m, float p[3], float c[3], float n[3], float t[2]){
	float *vert = malloc(sizeof(float) * 11);
	memcpy(vert, p, 3 * sizeof(float));
	memcpy(vert + 3, c, 3 * sizeof(float));
	memcpy(vert + 6, n, 3 * sizeof(float));
	memcpy(vert + 9, t, 2 * sizeof(float));
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

		/* Texure coords */
		*p++ = vert[9];
		*p++ = vert[10];

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
	int size = m->n_verticies * 11 * sizeof(float);
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

	//printf("mesh render, vertexId=%d, elementId=%d\n", m->vertexId, m->elementId);
	glBindBuffer(GL_ARRAY_BUFFER, m->vertexId);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 11 * sizeof(float), NULL);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 11 * sizeof(float), (void*) (3 * sizeof(float))); 

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 11 * sizeof(float), (void*) (6 * sizeof(float)));

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 11 * sizeof(float), (void*) (9 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->elementId);	

	textureset_bind();
	glDrawElements(
			GL_TRIANGLES, 
			m->n_trigs * 3,
			GL_UNSIGNED_INT,
			NULL
		      );

	textureset_unbind();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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
	tmp->ix = 0; tmp->iy = 0; tmp->iz = 0;
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
chunk_add_modified_block(chunk_t *c, int x, int y, int z){
	if(!c->modified) c->modified = 1;
	int *block_ind = malloc(sizeof(int) * 3);
	block_ind[0] = x;
	block_ind[1] = y;
	block_ind[2] = z;
	util_list_add(c->modified_list, block_ind);
}

void
chunk_remove_block(chunk_t *c, int w_x, int w_y, int w_z){
	int x, y, z;
	x = w_x % CHUNK_SIZE;
	y = w_y % CHUNK_SIZE;
	z = w_z % CHUNK_SIZE;
	c->blocks[x][y][z] = 0;
	/* Todo: Do this later */
	chunk_add_modified_block(c, x, y, z);
	chunk_rebuild(c);
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

	c->ix = x;
	c->iy = y;
	c->iz = z;

	c->pos[0] = x * (2 * CHUNK_SIZE);
	c->pos[1] = y * (2 * CHUNK_SIZE);
	c->pos[2] = z * (2 * CHUNK_SIZE);

	return 0;
}

/* TODO: Figure out a better data structure for looking up chunks by (ix,iy,iz) */
chunk_t*
chunkmanager_get_chunk(int ix, int iy, int iz){
	linked_list_elm_t *elm;

	elm = chunkmanager->loaded_chunks->head;
	while(elm != NULL){
		chunk_t *c = elm->data;
		if(c->ix == ix && c->iy == iy && c->iz == iz)
			return c;
		elm = elm->next;
	}
	return NULL;
}

void
chunkmanager_init(world_file_t *world){
	chunkmanager = malloc(sizeof(chunkmanager_t));
	chunkmanager->render_chunks = util_list_create();
	chunkmanager->loaded_chunks = util_list_create();
	chunkmanager->active_blocks = 0;
	chunkmanager->n_trigs = 0;
	
	for(int i = 0; i < (int)world->size[0] / CHUNK_SIZE; i++)
		for(int j = 0; j < (int)world->size[1] / CHUNK_SIZE; j++)
			for(int k = 0; k < (int)world->size[2] / CHUNK_SIZE; k++){
				chunk_t *c = chunk_create();
				world_read_chunk(world, i, j, k, c);
				util_list_add(chunkmanager->loaded_chunks, c);
			}
}

void
chunkmanager_rebuild(void){
	linked_list_elm_t *elm;

	elm = chunkmanager->loaded_chunks->head;
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

static int
should_chunk_be_rendered(chunk_t *c){
	/* Don't render emtpy chunks */
	if(c->active_blocks == 0)
		return 0;

	chunk_t *front = chunkmanager_get_chunk(c->ix, c->iy, c->iz+1);
	chunk_t *back = chunkmanager_get_chunk(c->ix, c->iy, c->iz-1);
	chunk_t *top = chunkmanager_get_chunk(c->ix, c->iy+1, c->iz);
	chunk_t *bottom = chunkmanager_get_chunk(c->ix, c->iy-1, c->iz);
	chunk_t *left = chunkmanager_get_chunk(c->ix-1, c->iy, c->iz);
	chunk_t *right = chunkmanager_get_chunk(c->ix+1, c->iy, c->iz);

	/* Don't render surrounded chunks */
	int s = front != NULL && front->active_blocks == MAX_ACTIVE_BLOCKS &&
		back  != NULL &&  back->active_blocks == MAX_ACTIVE_BLOCKS &&
		top   != NULL &&   top->active_blocks == MAX_ACTIVE_BLOCKS &&
		bottom!= NULL &&bottom->active_blocks == MAX_ACTIVE_BLOCKS &&
		left  != NULL &&  left->active_blocks == MAX_ACTIVE_BLOCKS &&
		right != NULL && right->active_blocks == MAX_ACTIVE_BLOCKS; 
	if(s)
		return 0;

	return 1;
}

static void
update_render_list(void){
	linked_list_elm_t *elm;

	/* Nuke render list */
	util_list_free(chunkmanager->render_chunks);
	chunkmanager->render_chunks = util_list_create();

	elm = chunkmanager->loaded_chunks->head;
	while(elm != NULL){
		chunk_t *c = elm->data;
		if(should_chunk_be_rendered(c))
			util_list_add(chunkmanager->render_chunks, c);	
		elm = elm->next;
	}
}

void
chunkmanager_update(void){
	update_render_list();
}

void
chunkmanager_free(void){
	util_list_free_custom(chunkmanager->loaded_chunks, chunk_free);
	util_list_free(chunkmanager->render_chunks);
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

		SDL_FreeSurface(image);
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, teximgs[1]->w, teximgs[1]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[1]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, teximgs[0]->w, teximgs[0]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[0]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, teximgs[2]->w, teximgs[2]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[2]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, teximgs[3]->w, teximgs[3]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[3]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, teximgs[4]->w, teximgs[4]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[4]->pixels);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, teximgs[5]->w, teximgs[5]->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, teximgs[5]->pixels);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for(int i = 0; i < 6; i++) SDL_FreeSurface(teximgs[i]);

	return textureId;
}

skybox_t*
skybox_create(void){
	skybox_t *sb = malloc(sizeof(skybox_t));
	sb->textureId = load_cubemap("skybox");

	return sb;
}

void
skybox_render(skybox_t *sb){
	renderblock_with_textures(camera->eye[0], camera->eye[1], camera->eye[2], sb->textureId);
}
