#pragma once

typedef struct CAT_ilist
{
	int* data;
	int capacity;
	int length;
} CAT_ilist;

void CAT_ilist_insert(CAT_ilist* list, int item, int idx);
int CAT_ilist_find(CAT_ilist* list, int item);
void CAT_ilist_remove(CAT_ilist* list, int idx);
void CAT_ilist_clear(CAT_ilist* list);

void CAT_ilist_push(CAT_ilist* list, int item);
int CAT_ilist_pop(CAT_ilist* list);
void CAT_ilist_enqueue(CAT_ilist* list, int item);
int CAT_ilist_dequeue(CAT_ilist* list);