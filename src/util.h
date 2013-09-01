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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <SDL/SDL.h>
#include <math.h>

#define PI 3.14159265359

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/* Logging */
#define FATAL_ERROR(...) (util_fatalerror(__FILE__,__LINE__,__VA_ARGS__))
#define LOG_DEBUG(...) (util_log_debug(__FILE__, __LINE__, __VA_ARGS__))
#define LOG_WARN(...) (util_log_warn(__FILE__, __LINE__, __VA_ARGS__))
#define LOG_FATAL_OR_DEBUG(abort, ...) do { if(abort) FATAL_ERROR(__VA_ARGS__); else LOG_DEBUG(__VA_ARGS__); } while(0)

void util_fatalerror(char *file, int line, char *fmt, ...);
void util_log_error(char *file, int line, char *fmt, ...);
void util_log_debug(char *file, int line, char *fmt, ...);
void util_log_warn(char *file, int line, char *fmt, ...);

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

enum {
	LIST_SORT_INC,
	LIST_SORT_DEC
}; 

linked_list_t *util_list_create(void);
void util_list_free(linked_list_t* l);
void util_list_free_data(linked_list_t* l);
void util_list_free_custom(linked_list_t *l, void (*freefunc)(void*));
void util_list_add(linked_list_t* lst, void *data); /* Append */
void util_list_insert(linked_list_t* lst, void *data); /* Insert first */
int util_list_insert_at(linked_list_t* lst, void *data, int ind); 
void* util_list_get(linked_list_t* lst, int ind); /* NB: Don't use this if performance is important */
int util_list_remove(linked_list_t* lst, void *data);
int util_list_size(linked_list_t *lst);
linked_list_t* util_list_sort(linked_list_t *l, int sort_order, int (*cmp)(void *a, void *b));

/* Math */
void crossproduct(double a[3], double b[3], double c[3]); 
double dotproduct(double a[3], double b[3]);
double length(double v[3]);
void normalize(double v[3]);
void rotatearoundYaxis(double v[3], double radians);
void rotatearoundXaxis(double v[3], double radians);
void rotatearoundZaxis(double v[3], double radians);
void vec_diff(double res[3], double a[3], double b[3]);
void vec_cpy(double d[3], double s[3]);
void vec_add(double u[3], double v[3]);
double deg2radians(double deg);

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
void quad_cpyvec(double v[3], quaternion_t *q);
void quad_fromvec(quaternion_t *q, double v[3]);

/* FPS independent Animation */
typedef struct anim_task_s {
	double units_per_second;
	double total_length;
	double current_length;
	int is_active;
	void (*update)(double scale_factor);
} anim_task_t;

void util_anim_update(Uint32 diff);
void util_anim_remove_anim_task(anim_task_t *b);
void util_anim_reset_anim_task(anim_task_t *t);
anim_task_t* util_anim_create(double ups, double total, int is_active, void (*update)(double factor));

/* Hash table */

typedef struct hashtable_s {
	int n_buckets;	
	linked_list_t **buckets;
} hashtable_t;

hashtable_t* util_hashtable_create(int n_buckets);
void util_hashtable_free(hashtable_t* ht);
void util_hashtable_free_data(hashtable_t* ht);
int util_hashtable_get(hashtable_t* ht, char *key, int key_len, void **out_data);
void util_hashtable_insert(hashtable_t* ht, char *key, int key_len, void *data);
int util_hashtable_remove(hashtable_t *ht, char *key, int key_len);

/* Settings manager */

void util_settings_load_default_files(void);
int util_settings_load_file(char *inifile);
int util_settings_remove(char *property);
int util_settings_geti(char *property, int *out);
int util_settings_getb(char *property, int *out);
int util_settings_getf(char *property, float *out);
int util_settings_gets(char *property, char **out);

/* These functions aborts the program if the property doesn't exist */
int util_settings_polli(char *property);
int util_settings_pollb(char *property);
float util_settings_pollf(char *property);
char* util_settings_polls(char *property);

#endif
