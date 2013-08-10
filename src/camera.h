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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#define NEAR_PLANE 0.1
#define FAR_PLANE 1000.0

typedef struct camera_s {
	double eye[3];

	quaternion_t forward;
	quaternion_t up;
} camera_t;

extern camera_t *camera;

void camera_create(void);
void camera_load_perspective(void);
void camera_load_modelview(void);
void camera_free(void);
void camera_move(double x, double y, double z);

#endif
