#include "cat_structures.h"

#include <limits.h>
#include "cat_math.h"
#include <string.h>
#include "assert.h"

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

void CAT_ilist_sort(CAT_int_list* list)
{
	int i = 1;
	while(i < list->length)
	{
		int key = list->data[i];
		int j = i;
		while(j > 0 && list->data[j-1] > key)
		{
			list->data[j] = list->data[j-1];
			j -= 1;
		}
		list->data[j] = key;
		i += 1;
	}
}

void CAT_ilist_sort_by_proc(CAT_int_list* list, int (*cmp) (int, int))
{
	int i = 1;
	while(i < list->length)
	{
		int key = list->data[i];
		int j = i;
		while(j > 0 && cmp(list->data[j-1], key) > 1)
		{
			list->data[j] = list->data[j-1];
			j -= 1;
		}
		list->data[j] = key;
		i += 1;
	}
}

typedef struct
{
	void* backing;
	uint16_t backing_size;
	uint8_t item_size;

	uint16_t item_capacity;
	uint16_t item_count;
} CAT_list;

#define LIST_MAX_ITEM_SIZE 64

void CAT_list_init(CAT_list* list, void* backing, uint16_t backing_size, uint8_t item_size)
{
	list->backing = backing;
	list->backing_size = backing_size;
	list->item_size = CAT_min(item_size, LIST_MAX_ITEM_SIZE);
	
	list->item_capacity = list->backing_size/list->item_size;
	list->item_count = 0;
}

void CAT_list_clear(CAT_list* list)
{
	list->item_count = 0;
}

#define LIST_GET(l, i) (l->backing + l->item_size * (i))
#define LIST_READ(l, i, dst) memcpy((dst), LIST_GET((l), (i)), l->item_size)
#define LIST_WRITE(l, i, src) memcpy(LIST_GET((l), (i)), (src), l->item_size)
#define LIST_COPY(l, i, j) memcpy(LIST_GET((l), (i)), LIST_GET((l), (j)), l->item_size)

bool CAT_list_insert(CAT_list* list, int idx, void* src)
{
	if(list->item_count >= list->item_capacity)
		return false;
	if(idx < 0 || idx > list->item_count)
		return false;
	for(int i = list->item_count; i > idx; i--)
		LIST_COPY(list, i, i-1);
	LIST_WRITE(list, idx, src);
	list->item_count += 1;
	return true;
}

bool CAT_list_delete(CAT_list* list, int idx, void* dst)
{
	if(idx < 0 || idx >= list->item_count)
		return false;
	list->item_count -= 1;
	for(int i = idx; i < list->item_count; i++)
		LIST_COPY(list, i, i+1);
	LIST_READ(list, idx, dst);
	return true;
}

uint8_t list_temp_buffer[LIST_MAX_ITEM_SIZE];
bool CAT_list_swap(CAT_list* list, int i, int j)
{
	if(i < 0 || i >= list->item_count)
		return false;
	if(j < 0 || j >= list->item_count)
		return false;
	uint8_t* temp[list->item_size];
	LIST_READ(list, i, temp);
	LIST_COPY(list, i, j);
	LIST_WRITE(list, j, temp);
	return true;
}

void CAT_list_shuffle(CAT_list* list)
{
	for(int i = 0; i < list->item_count; i++)
	{
		int j = CAT_rand_int(0, list->item_count-1);
		CAT_list_swap(list, i, j);
	}
}

int CAT_list_push(CAT_list* list, void* src)
{
	int idx = list->item_count;
	if(CAT_list_insert(list, idx, src))
		return idx;
	return -1;
}

bool CAT_list_pop(CAT_list* list, void* dst)
{
	return CAT_list_delete(list, list->item_count-1, dst);
}

bool CAT_list_enqueue(CAT_list* list, void* src)
{
	return CAT_list_insert(list, 0, src);
}

bool CAT_list_dequeue(CAT_list* list, void* dst)
{
	return CAT_list_delete(list, 0, dst);
}


