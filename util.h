#ifndef __UTIL_H__
#define __UTIL_H__

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

#endif
