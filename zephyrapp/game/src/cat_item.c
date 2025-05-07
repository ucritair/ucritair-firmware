#include "cat_item.h"

#include <stdio.h>
#include "cat_render.h"
#include <string.h>
#include <stddef.h>

//////////////////////////////////////////////////////////////////////////
// ITEM TABLE

CAT_item* CAT_item_get(int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		return NULL;
	}
		
	return &item_table.data[item_id];
}


//////////////////////////////////////////////////////////////////////////
// ITEM LIST

void CAT_item_list_init(CAT_item_list* item_list)
{
	item_list->length = 0;
}

int CAT_item_list_find(CAT_item_list* item_list, int item_id)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return -1;
	}

	for(int i = 0; i < item_list->length; i++)
	{
		if(item_list->item_ids[i] == item_id)
		{
			return i;
		}
	}
	return -1;
}

int CAT_item_list_add(CAT_item_list* item_list, int item_id, int count)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return -1 ;
	}
	if(item_list->length >= CAT_ITEM_LIST_MAX_LENGTH)
	{
		CAT_printf("[WARNING] attempted add to full item list\n");
		return -1;
	}
		
	int idx = CAT_item_list_find(item_list, item_id);
	if(idx >= 0)
	{
		item_list->counts[idx] += count;
		return idx;
	}
	else
	{
		// REMOVED ALPHABETICAL SORTING TO SIMPLIFY ITEM LIST GUI LOGIC
		/* const char* a = item_table.data[item_id].name;
		int insert_idx = 0;
		while(insert_idx < item_list->length)
		{
			const char* b = item_table.data[item_list->item_ids[insert_idx]].name;
			if(strcmp(a, b) < 0)
				break;
			insert_idx += 1;
		}
		for(int i = item_list->length; i > insert_idx; i--)
		{
			item_list->item_ids[i] = item_list->item_ids[i-1];
			item_list->counts[i] = item_list->counts[i-1];
		} */

		int insert_idx = item_list->length;
		item_list->item_ids[insert_idx] = item_id;
		item_list->counts[insert_idx] = count;
		item_list->length += 1;
		return insert_idx;
	}
}

void CAT_item_list_remove(CAT_item_list* item_list, int item_id, int count)
{
	if(item_id < 0 || item_id >= item_table.length)
	{
		CAT_printf("[ERROR] reference to invalid item id: %d\n", item_id);
		return;
	}

	int idx = CAT_item_list_find(item_list, item_id);
	if(idx >= 0)
	{
		if(count == -1)
			count = item_list->counts[idx];
		item_list->counts[idx] -= count;
		if(item_list->counts[idx] <= 0)
		{
			for(int i = idx; i < item_list->length-1; i++)
			{
				item_list->item_ids[i] = item_list->item_ids[i+1];
				item_list->counts[i] = item_list->counts[i+1];
			}
			item_list->length -= 1;
		}
	}
}

void CAT_item_list_filter(CAT_item_list* a, CAT_item_list* b, CAT_item_filter filter)
{
	b->length = 0;
	for(int i = 0; i < a->length; i++)
	{
		int candidate_id = a->item_ids[i];
		if(CAT_item_get(candidate_id) == NULL)
			continue;
		int candidate_count = a->counts[i];
		if(candidate_count <= 0)
			continue;
		if(filter != NULL && !filter(a->item_ids[i]))
			continue;
		CAT_item_list_add(b, candidate_id, candidate_count);
	}
}