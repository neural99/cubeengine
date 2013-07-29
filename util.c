#include <stdio.h>
#include <stdlib.h>
#include "util.h"

linked_list_t*
util_list_create(void){
	linked_list_t *l = malloc(sizeof(linked_list_t));
	l->head = NULL;
	l->last = NULL;
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


