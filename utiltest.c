#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int 
main(int argc, char *argv[]){
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

	hashtable_t *ht = util_hashtable_create(200);

	for(int i = 0; i < 100; i++){
		char buff[3];
		snprintf(buff, 3, "%d", i);
		util_hashtable_insert(ht, buff, 2, buff); 
	}

	char *d1= "11";
	char *d2= "7";
	util_hashtable_remove(ht, d1, 2);
	util_hashtable_remove(ht, d2, 2);

	for(int i = 0; i < 10; i++){
		char *s;
		char buff[3];
		snprintf(buff, 3, "%d", i);
		if(util_hashtable_get(ht, buff, 2, &s))
			printf("i = %d, s = %s\n", i, s);
		else
			printf("i = %d not found", i);
	} 

	return 0;
}
