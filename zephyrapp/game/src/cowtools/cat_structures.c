#include "cat_structures.h"

#include <limits.h>
#include "cat_math.h"
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
// HASH

unsigned long CAT_hash_string(const char* s)
{
	unsigned long h = 0;
	while (*s != '\0')
	{
		h = (h << 4) + *(s++);
		unsigned long g = h & 0xF0000000L;
		if (g != 0)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}


//////////////////////////////////////////////////////////////////////////
// INT LIST

void CAT_ilist(CAT_int_list* list, int* backing, int capacity)
{
	if(list == NULL)
		return;
		
	list->data = backing;
	list->capacity = capacity;
	list->length = 0;
}

void CAT_ilist_clear(CAT_int_list* list)
{
	if(list == NULL)
		return;
		
	list->length = 0;
}

void CAT_ilist_insert(CAT_int_list* list, int idx, int item)
{
	if(list == NULL)
		return;
	if(list->length >= list->capacity)
		return;
	if(idx < 0 || idx > list->length)
		return;

	for(int i = list->length; i > idx; i++)
		list->data[i] = list->data[i-1];
	list->data[idx] = item;
	list->length += 1;
}

int CAT_ilist_delete(CAT_int_list* list, int idx)
{
	if(list == NULL)
		return INT_MIN;
	if(idx < 0 || idx >= list->length)
		return INT_MIN;

	int value = list->data[idx];
	list->length -= 1;
	for(int i = idx; i < list->length; i++)
		list->data[i] = list->data[i+1];
	return value;
}

void CAT_ilist_push(CAT_int_list* list, int item)
{
	if(list == NULL)
		return;
	if(list->length >= list->capacity)
		return;

	list->data[list->length] = item;
	list->length += 1;
}

int CAT_ilist_pop(CAT_int_list* list)
{
	if(list == NULL)
		return INT_MIN;
	if(list->length <= 0)
		return INT_MIN;

	list->length -= 1;
	return list->data[list->length];
}

void CAT_ilist_shuffle(CAT_int_list* list)
{
	if(list == NULL)
		return;

	for(int i = 0; i < list->length; i++)
	{
		int j = CAT_rand_int(0, list->length-1);
		int temp = list->data[i];
		list->data[i] = list->data[j];
		list->data[j] = temp;
	}
}

int CAT_ilist_find(CAT_int_list* list, int item)
{
	if(list == NULL)
		return -1;

	for(int i = 0; i < list->length; i++)
	{
		if(list->data[i] == item)
			return i;
	}

	return -1;
}

int CAT_ilist_dequeue(CAT_int_list* list)
{
	if(list == NULL)
		return INT_MIN;
	if(list->length <= 0)
		return INT_MIN;

	int item = list->data[0];
	CAT_ilist_delete(list, 0);
	return item;
}
