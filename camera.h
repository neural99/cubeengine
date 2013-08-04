#ifndef __CAMERA_H__
#define __CAMERA_H__

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
