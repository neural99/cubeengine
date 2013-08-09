#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"
#include "startup.h"

#ifdef WIN32
#include <windows.h>
#endif

#define SETTINGS_TABLE_SIZE 2048

typedef enum {
	SETTING_INT,
	SETTING_FLOAT,
	SETTING_BOOL,
	SETTING_STRING
} setting_type_t;

typedef struct setting_entry_s {
	setting_type_t type;
	int intval;
	float floatval;
	char *strval;
} setting_entry_t;

char *setting_type_names[]  = { "Integer", "Float", "Bool", "String" };

typedef struct hashtable_bucket_elm_s {
	char *key;
	int key_len;
	void *data;
} hashtable_bucket_elm_t;

static linked_list_t *animation_list = NULL;
static hashtable_t *settings_table = NULL;

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
util_log_warn(char *file, int line, char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	char buff2[500];
	vsprintf(buff2, fmt, va);
	printf("Warning(%s:%d): %s\n", file, line, buff2);
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
util_list_free_custom(linked_list_t *lst, void (*freefunc)(void*)){
	linked_list_elm_t *elm;

	elm = lst->head;
	while(elm != NULL){
		linked_list_elm_t *next = elm->next;
		freefunc(elm->data);
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

void
util_list_insert(linked_list_t *lst, void *data){
	linked_list_elm_t *elm;

	elm = malloc(sizeof(linked_list_elm_t));

	elm->data = data;
	elm->next = lst->head;
	lst->head = elm;
}

void*
util_list_get(linked_list_t *lst, int ind){
	int i = 0;
	linked_list_elm_t *elm;
	elm = lst->head;
	while(elm != NULL){
		if(i == ind)
			return elm->data;	
		i++;
		elm = elm->next;
	}
	return NULL;
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
			if(task->total_length > 0){
				if(units_this_frame + task->current_length < task->total_length){
					task->current_length += units_this_frame;
					task->update(units_this_frame);
				}else{
					double units_left = task->total_length - task->current_length;
					task->update(units_left);
					task->is_active = 0;
					task->current_length = 0;
				}
			}else{
				task->update(units_this_frame);
			}
		}
		elm = elm->next;
	}
}

static void 
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

void
vec_diff(double res[3], double a[3], double b[3]){
	res[0] = a[0] - b[0];
	res[1] = a[1] - b[1];
	res[2] = a[2] - b[2];
}


/* Java's method of calculating String hashes */
static unsigned int
hashfunction(char *key, int key_len){
	int k = 31;	
	int a = 0;

	char *last = key + key_len;
	int i = 0;
	while(last != key){
		a += k * *last;
		k *= k;
		i++;
		last--;
	}
	return a;	
}

static int 
keyequals(char *key1, int key1_len, char *key2, int key2_len){
	if(key1_len != key2_len)
		return 0;
	for(int i = 0; i < key1_len; i++)
		if(key1[i] != key2[i])
			return 0;
	return 1;
}

hashtable_t*
util_hashtable_create(int n_buckets){
	hashtable_t *ht = malloc(sizeof(hashtable_t));
	ht->n_buckets = n_buckets;
	ht->buckets = malloc(n_buckets * sizeof(linked_list_t*));
	for(int i = 0; i < n_buckets; i++)
		ht->buckets[i] = util_list_create();
	return ht;
}

void
util_hashtable_free(hashtable_t *h){
	for(int i = 0; i < h->n_buckets; i++){
		/* First free every bucket elment in bucket */
		linked_list_elm_t *elm;
		elm = h->buckets[i]->head;
		while(elm != NULL){
			hashtable_bucket_elm_t *be = elm->data;
			free(be->key);
			free(be);
			elm = elm->next;
		}
		/* Then free list */
		util_list_free(h->buckets[i]);
	}
	free(h->buckets);
	free(h);
}

void
util_hashtable_free_data(hashtable_t *h){
	/* First call free on data pointers in the bucket elements */
	for(int i = 0; i < h->n_buckets; i++){
		linked_list_elm_t *elm;
		elm = h->buckets[i]->head;
		while(elm != NULL){
			hashtable_bucket_elm_t *be = elm->data;
			free(be->data);
		}

	}
	util_hashtable_free(h);
}

int 
util_hashtable_get(hashtable_t *ht, char *key, int key_len, void **out_data){
	unsigned int ind = hashfunction(key, key_len) % ht->n_buckets;
	linked_list_t *bucket = ht->buckets[ind];

	linked_list_elm_t *elm;
	elm = bucket->head;
	while(elm != NULL){
		hashtable_bucket_elm_t *be = elm->data;
		if(keyequals(key, key_len, be->key, be->key_len)){
			*out_data = be->data;
			return 1;
		}
		elm = elm->next;
	}

	*out_data = NULL;
	return 0;
}

void
util_hashtable_insert(hashtable_t *ht, char *key, int key_len, void *data){
	unsigned int ind = hashfunction(key, key_len) % ht->n_buckets;
	linked_list_t *bucket = ht->buckets[ind];

	hashtable_bucket_elm_t *be = malloc(sizeof(hashtable_bucket_elm_t));	
	be->key = malloc(key_len);
	memcpy(be->key, key, key_len);
	be->key_len = key_len;
	be->data = data;

	util_list_add(bucket, be);
}

int 
util_hashtable_remove(hashtable_t *ht, char *key, int key_len){
	unsigned int ind = hashfunction(key, key_len) % ht->n_buckets;
	linked_list_t *bucket = ht->buckets[ind];

	/* Look for bucket elm with matching key */
	linked_list_elm_t *elm;
	elm = bucket->head;
	hashtable_bucket_elm_t *match = NULL;
	while(elm != NULL){
		hashtable_bucket_elm_t *be = elm->data;
		if(keyequals(key, key_len, be->key, be->key_len)){
			match = be;
			break;
		}
		elm = elm->next;
	}

	if(match != NULL){
		free(match->key);
		free(match);
	}
	return util_list_remove(bucket, match);
}


int
util_settings_remove(char *prop){
	setting_entry_t *entry;
	int r = util_hashtable_get(settings_table, prop, strlen(prop), (void**) &entry);
	if(r){
		if(entry->type == SETTING_STRING)
			free(entry->strval);
		free(entry);
		util_hashtable_remove(settings_table, prop, strlen(prop));
		return 1;
	}else{
		return 0;
	}
}

static void
add_integer_settings(char *name, int val){
	/* First remove hashtable entry if exists */
	util_settings_remove(name);

	setting_entry_t *entry = malloc(sizeof(setting_entry_t));
	entry->type = SETTING_INT;
	entry->intval = val;

	util_hashtable_insert(settings_table, name, strlen(name), entry);
}

static void
add_float_settings(char *name, float val){
	/* First remove hashtable entry if exists */
	util_settings_remove(name);

	setting_entry_t *entry = malloc(sizeof(setting_entry_t));
	entry->type = SETTING_FLOAT;
	entry->floatval = val;
	
	util_hashtable_insert(settings_table, name, strlen(name), entry);
}

static void 
add_string_settings(char *name, char *val){
	/* First remove hashtable entry if exists */
	util_settings_remove(name);

	setting_entry_t *entry = malloc(sizeof(setting_entry_t));
	entry->strval = malloc(strlen(val) + 1);
	entry->type = SETTING_STRING;
	strcpy(entry->strval, val);

	util_hashtable_insert(settings_table, name, strlen(name), entry);
}

static void 
add_bool_settings(char *name, int val){
	/* First remove hashtable entry if exists */
	util_settings_remove(name);

	setting_entry_t *entry = malloc(sizeof(setting_entry_t));
	entry->type = SETTING_BOOL;
	entry->intval = val;

	util_hashtable_insert(settings_table, name, strlen(name), entry);
}

int 
util_settings_load_file(char *inifile){
	if(settings_table == NULL)
		settings_table = util_hashtable_create(SETTINGS_TABLE_SIZE);

#ifdef WIN32
	char path[MAX_PATH];
	ExpandEnvironmentStrings(inifile, path, MAX_PATH);
#else
	char *path = inifile;
#endif

	FILE *f = fopen(path, "r");
	if(f == NULL)
		return -1;
	int lines = 0;
	char buff[2048];
	char name[2048];
	char *str;
	int dec;
	float flt;
	while(!feof(f)){
		char *c = fgets(buff, 2048, f);
		if(c == NULL)
			break;

		/* Copy name */
		char *a = buff;
		char *b = name;
		while(*a != '=' && *a != 0) 
			*b++ = *a++;
		if(*a == 0)
			FATAL_ERROR("Error parsing inifile %s near line %d. End of line reached before '=' was found", path, lines+1);
		*b++ = 0;
		str = a + 1;

		int items;
		items = sscanf(str, "%d", &dec);
		if(items == 1){
			add_integer_settings(name, dec);
			/* Next line */
			lines++;
			continue;
		}

		items = sscanf(str, "%f", &flt);
		if(items == 1){
			add_float_settings(name, flt);
			/* Next line */
			lines++;
			continue;
		}

		/* Treat True and False as boolean values */
		int tru = strcmp(str, "True\n") == 0 || strcmp(str, "true\n") == 0;
	        int fal = strcmp(str, "False\n") == 0 || strcmp(str, "false\n") == 0;
		if(tru){
			add_bool_settings(name, 1);
			lines++;
			continue;
		}
		if(fal){
			add_bool_settings(name, 0);
			lines++;
			continue;
		}

		/* Remove '\n' */
		char *p1 = str;
		while(*p1 != 0 && *p1 != '\n') p1++;
		if(*p1 != 0) *p1 = 0;

		add_string_settings(name, str);

		lines++;
	}

	return lines;
}

int 
util_settings_geti(char *prop, int *out){
	if(settings_table == NULL)
		settings_table = util_hashtable_create(SETTINGS_TABLE_SIZE);
	setting_entry_t *entry;
	int r = util_hashtable_get(settings_table, prop, strlen(prop), (void**) &entry);
	if(r && entry->type == SETTING_INT){
		*out = entry->intval;
		return 0;
	}
	
	if(r)
		LOG_DEBUG("Setting %s was accessed as a int, but setting type is %s", prop, setting_type_names[entry->type]);
	else
		LOG_DEBUG("Setting %s doesn not exist in settings table", prop);

	return -1;
}

int 
util_settings_getb(char *prop, int *out){
	if(settings_table == NULL)
		settings_table = util_hashtable_create(SETTINGS_TABLE_SIZE);
	setting_entry_t *entry;
	int r = util_hashtable_get(settings_table, prop, strlen(prop), (void**) &entry);
	if(r && entry->type == SETTING_BOOL){
		*out = entry->intval;
		return 0;
	}
	
	if(r)
		LOG_DEBUG("Setting %s was accessed as a bool, but setting type is %s", prop, setting_type_names[entry->type]);
	else
		LOG_DEBUG("Setting %s doesn not exist in settings table", prop);

	return -1;
}

int
util_settings_getf(char *prop, float *out){
	if(settings_table == NULL)
		settings_table = util_hashtable_create(SETTINGS_TABLE_SIZE);
	setting_entry_t *entry;
	int r = util_hashtable_get(settings_table, prop, strlen(prop), (void**) &entry);
	if(r && entry->type == SETTING_FLOAT){
		*out = entry->floatval;
		return 0;
	}
	
	if(r)
		LOG_DEBUG("Setting %s was accessed as a float, but setting type is %s", prop, setting_type_names[entry->type]);
	else
		LOG_DEBUG("Setting %s doesn not exist in settings table", prop);

	return -1;
}

int
util_settings_gets(char *prop, char** out){
	if(settings_table == NULL)
		settings_table = util_hashtable_create(SETTINGS_TABLE_SIZE);
	setting_entry_t *entry;
	int r = util_hashtable_get(settings_table, prop, strlen(prop), (void**) &entry);
	if(r && entry->type == SETTING_STRING){
		*out = entry->strval;
		return 0;
	}
	
	if(r)
		LOG_DEBUG("Setting %s was accessed as a string, but setting type is %s", prop, setting_type_names[entry->type]);
	else
		LOG_DEBUG("Setting %s doesn not exist in settings table", prop);

	return -1;
}

void
util_settings_load_default_files(void){
	if(util_settings_load_file("defaults.ini") < 0)
		FATAL_ERROR("Could not load defaults settings from defaults.ini");

	/* TODO: Load ini file from users home dir on *nix */
#ifdef WIN32
	if(util_settings_load_file("%APPDATA%/cubeengine/settings.ini") < 0)
		LOG_WARN("Could not read settings from user settings file");
#endif

}
STARTUP_PROC(settings, 0, util_settings_load_default_files)
