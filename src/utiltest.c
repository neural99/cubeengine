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
