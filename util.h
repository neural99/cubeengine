#ifndef __UTIL_H__
#define __UTIL_H__

#define PI 3.14159265359

/* Logging */
#define FATAL_ERROR(str) (util_fatalerror(__FILE__,__LINE__,str))

void util_fatalerror(char *file, int line, char *msg);
void util_log_error(char *file, int line, char *msg);

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

#endif
