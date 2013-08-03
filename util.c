#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"

static linked_list_t *animation_list = NULL;

void
util_fatalerror(char *file, int line, char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	char buff2[500];
	vsprintf(buff2, fmt, va);
	printf("Error(%s:%d): %s\n", file, line, buff2);
	va_end(va);
	exit(1);
}

void
util_log_error(char *file, int line, char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	char buff2[500];
	vsprintf(buff2, fmt, va);
	printf("Error(%s:%d): %s\n", file, line, buff2);
	va_end(va);
}

void
util_log_debug(char *file, int line, char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	char buff2[500];
	vsprintf(buff2, fmt, va);
#ifdef DEBUG
	printf("Debug(%s:%d): %s\n", file, line, buff2);
#endif
	va_end(va);
}

linked_list_t*
util_list_create(void){
	linked_list_t *l = malloc(sizeof(linked_list_t));
	l->head = NULL;
	l->last = NULL;
	return l;
}

void
util_list_free(linked_list_t *lst){
	linked_list_elm_t *elm;

	elm = lst->head;
	while(elm != NULL){
		linked_list_elm_t *next = elm->next;
		free(elm);
		elm = next;
	}

	free(lst);
}

void
util_list_free_data(linked_list_t *lst){
	linked_list_elm_t *elm;

	elm = lst->head;
	while(elm != NULL){
		linked_list_elm_t *next = elm->next;
		free(elm->data);
		free(elm);
		elm = next;
	}

	free(lst);
}

void 
util_list_add(linked_list_t *lst, void *data){
	linked_list_elm_t* elm; 

	elm = malloc(sizeof(linked_list_elm_t));

	elm->data = data;
	elm->next = NULL;
	
	if(lst->head == NULL){
		lst->head = elm;
		lst->last = elm;
	}else{
		lst->last->next = elm;
		lst->last = elm;
	}
}

int
util_list_remove(linked_list_t *lst, void *data){
	linked_list_elm_t *curr, *prev, *next;
	linked_list_elm_t *match;

	match = NULL;
	prev = NULL;
	curr = lst->head;

	while(curr != NULL){
		if(curr->data == data){
			match = curr;
			break;
		}
		
		prev = curr;
		curr = curr->next;
	}

	if(match){
		next = match->next;

		if(prev == NULL){
			if(next == NULL){
				lst->last = NULL;
				lst->head = NULL;
			}else{
				lst->head = next;
			}
		}else{
			if(next == NULL){
				prev->next = NULL;
				lst->last = prev;
			}else{
				prev->next = next;	
			}
		}

		free(match);
		return 1;
	}
	return 0;
}

int
util_list_size(linked_list_t *lst){
	linked_list_elm_t *elm;
	int size;

	size = 0;	
	elm = lst->head;
	while(elm != NULL) {
		size++;
		elm = elm->next;
	}
	return size;
}	

void
crossproduct(double a[3], double b[3], double c[3]){
	a[0] = b[1] * c[2] - b[2] * c[1];
	a[1] = b[2] * c[0] - b[0] * c[2];
	a[2] = b[0] * c[1] - b[1] * c[0];
}

double
length(double v[3]){
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void
normalize(double v[3]){
	double len = length(v);
	if(len == 0) return;
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}

void
rotatearoundYaxis(double v[3], double radians){
	v[0] = v[0] * cos(radians) + v[2] * sin(radians);
	v[1] = v[1];
	v[2] = -v[0] * sin(radians) + v[2] * cos(radians);
}

void
rotatearoundXaxis(double v[3], double radians){
	v[0] = v[0];
	v[1] = v[1] * cos(radians)  - v[2] * sin(radians);
	v[2] = v[1] * sin(radians)  + v[2] * cos(radians);
}

void
rotatearoundZaxis(double v[3], double radians){
	v[0] = v[0] * cos(radians) - v[1] * sin(radians);
	v[1] = v[0] * sin(radians) + v[1] * cos(radians);
	v[2] = v[2];
}

void
util_anim_update(Uint32 diff){
	linked_list_elm_t *elm;

	elm = animation_list->head;
	while(elm != NULL){
		anim_task_t *task = elm->data;
		if(task->is_active){
			double units_this_frame = task->units_per_second * diff / 1000;
			if(units_this_frame + task->current_length < task->total_length){
				task->current_length += units_this_frame;
				task->update(units_this_frame);
			}else{
				double units_left = task->total_length - task->current_length;
				task->update(units_left);
				task->is_active = 0;
				task->current_length = 0;
			}

		}
		elm = elm->next;
	}
}

void 
util_anim_add_anim_task(anim_task_t *task){
	if(animation_list == NULL)
		animation_list = util_list_create();
	util_list_add(animation_list, task);
}

void
util_anim_remove_anim_task(anim_task_t *task){
	util_list_remove(animation_list, task);
	if(util_list_size(animation_list) == 0){
		util_list_free(animation_list);
		animation_list = NULL;
	}
}

anim_task_t*
util_anim_create(double ups, double total, int is_active, void (*update)(double factor)){
	anim_task_t *tmp = malloc(sizeof(anim_task_t));
	tmp->units_per_second = ups;
	tmp->total_length = total;
	tmp->current_length = 0;
	tmp->is_active = is_active;
	tmp->update = update;
	util_anim_add_anim_task(tmp);
	return tmp;
}

void
util_anim_reset_anim_task(anim_task_t *t){
	t->is_active = 1;
	t->current_length = 0;
}

double
quad_length(quaternion_t *q){
	return sqrt(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

double
quad_diff(quaternion_t *q1, quaternion_t *q2){
	double dx = q2->x - q1->x;
	double dy = q2->y - q1->y;
	double dz = q2->z - q1->z;
	double dw = q2->w - q1->w;
	return sqrt(dx*dx + dy*dy + dz*dz + dw*dw);
}

void
quad_normalize(quaternion_t *q){
	int len = quad_length(q);
	if(len != 0){
		q->x = q->x / len;
		q->y = q->y / len;
		q->z = q->z / len;
		q->w = q->w / len;
	}
}

void
quad_conjugate(quaternion_t *q){
	q->x = -q->x;
	q->y = -q->y;
	q->z = -q->z;
}

void 
quad_mult(quaternion_t *res, quaternion_t *a, quaternion_t *b){
	res->x = a->w*b->x + a->x*b->w + a->y*b->z - a->z*b->y;
	res->y = a->w*b->y - a->x*b->z + a->y*b->w + a->z*b->x;
	res->z = a->w*b->z + a->x*b->y - a->y*b->x + a->z*b->w;
	res->w = a->w*b->w - a->x*b->x - a->y*b->y - a->z*b->z;
}

void
quad_rotate(quaternion_t *q, double angle, quaternion_t *axis){
	q->x = axis->x * sin(angle/2);
	q->y = axis->y * sin(angle/2);
	q->z = axis->z * sin(angle/2);
	q->w = cos(angle/2);
}

void
quad_applyrotation(quaternion_t *res, quaternion_t *rot){
	quaternion_t conj_rot;
	memcpy(&conj_rot, rot, sizeof(quaternion_t));
	quad_conjugate(&conj_rot);

	quaternion_t tmp;
	quad_mult(&tmp, rot, res);
	quad_mult(res, &tmp, &conj_rot);
}
