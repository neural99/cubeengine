#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"


void
util_fatalerror(char *file, int line, char *msg){
	util_log_error(file, line, msg);
	exit(1);
}

void
util_log_error(char *file, int line, char *msg){
	printf("%s line %d: %s\n", file, line, msg);
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
