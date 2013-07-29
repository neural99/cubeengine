#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int 
main(void){
	int buf[100];
	linked_list_t* lst;

	lst = util_list_create();

	for (int i = 0; i < 100; i++){
		buf[i] = i;

		util_list_add(lst, &buf[i]);
	}

	linked_list_elm_t *elm;
	elm = lst->head;
	while(elm != NULL){
		int* d = elm->data;
		printf("%d ", *d);
		elm = elm->next;
	}
	puts("");

	for(int i = 0; i < 100; i++)
		if(i % 3)
			util_list_remove(lst, &buf[i]);

	for(int i = 0; i < 10; i++){
		util_list_add(lst, &buf[i]);
	}

	elm = lst->head;
	while(elm != NULL){
		int* d = elm->data;
		printf("%d ", *d);
		elm = elm->next;
	}

	puts("");

	util_list_free(lst);

	return 0;
}
