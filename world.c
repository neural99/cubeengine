#include <SDL/sdl.h>
#include <GL/gl.h>

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

