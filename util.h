#ifndef __UTIL_H__
#define __UTIL_H__

#include <SDL/sdl.h>

#define PI 3.14159265359

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/* Logging */
#define FATAL_ERROR(...) (util_fatalerror(__FILE__,__LINE__,__VA_ARGS__))
#define LOG_DEBUG(...) (util_log_debug(__FILE__, __LINE__, __VA_ARGS__))

void util_fatalerror(char *file, int line, char *fmt, ...);
void util_log_error(char *file, int line, char *fmt, ...);
void util_log_debug(char *file, int line, char *fmt, ...);

/* Linked list */
struct linked_list_s;
typedef struct linked_list_elm_s {
	void *data;
	struct linked_list_elm_s *next;
} linked_list_elm_t;

typedef struct linked_list_s {
	linked_list_elm_t *head;
	linked_list_elm_t *last;
} linked_list_t;

linked_list_t *util_list_create(void);
void util_list_free(linked_list_t* l);
void util_list_free_data(linked_list_t* l);
void util_list_free_custom(linked_list_t *l, void (*freefunc)(void*));
void util_list_add(linked_list_t* lst, void *data);
int util_list_remove(linked_list_t* lst, void *data);
int util_list_size(linked_list_t *lst);

/* Math */
void crossproduct(double a[3], double b[3], double c[3]); 
double length(double v[3]);
void normalize(double v[3]);
void rotatearoundYaxis(double v[3], double radians);
void rotatearoundXaxis(double v[3], double radians);
void rotatearoundZaxis(double v[3], double radians);

typedef struct quaternion_s {
	double x, y, z, w;
} quaternion_t;

double quad_length(quaternion_t *q);
void quad_normalize(quaternion_t *q);
void quad_mult(quaternion_t *res, quaternion_t *a, quaternion_t *b);
void quad_conjugate(quaternion_t *q);
void quad_rotate(quaternion_t* q, double angle, quaternion_t *axis);
void quad_applyrotation(quaternion_t *res, quaternion_t *rot);
double quad_diff(quaternion_t *q1, quaternion_t *q2);

/* FPS independent Animation */
typedef struct anim_task_s {
	double units_per_second;
	double total_length;
	double current_length;
	int is_active;
	void (*update)(double scale_factor);
} anim_task_t;

void util_anim_update(Uint32 diff);
void util_anim_add_anim_task(anim_task_t *a);
void util_anim_remove_anim_task(anim_task_t *b);
void util_anim_reset_anim_task(anim_task_t *t);
anim_task_t* util_anim_create(double ups, double total, int is_active, void (*update)(double factor));

#endif
