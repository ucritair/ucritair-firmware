#pragma once

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////
// HASH

unsigned long CAT_hash_string(const char* s);


//////////////////////////////////////////////////////////////////////////
// INT LIST

typedef struct
{
	int* data;
	int capacity;
	int length;
} CAT_int_list;

void CAT_ilist(CAT_int_list* list, int* backing, int capacity);
void CAT_ilist_clear(CAT_int_list* list);
void CAT_ilist_insert(CAT_int_list* list, int idx, int item);
int CAT_ilist_delete(CAT_int_list* list, int idx);
void CAT_ilist_push(CAT_int_list* list, int item);
int CAT_ilist_pop(CAT_int_list* list);
void CAT_ilist_shuffle(CAT_int_list* list);
int CAT_ilist_find(CAT_int_list* list, int item);
int CAT_ilist_dequeue(CAT_int_list* list);
int CAT_ilist_sort(CAT_int_list* list);
int CAT_ilist_sort_by_proc(CAT_int_list* list, int (*cmp) (int, int));
