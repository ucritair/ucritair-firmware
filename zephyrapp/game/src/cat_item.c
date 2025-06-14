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
		return NULL;
	return &item_table.data[item_id];
}

void CAT_filter_item_table(CAT_item_filter filter, CAT_int_list* list)
{
	for(int i = 0; i < item_table.length; i++)
	{
		if(filter == NULL || filter(i))
			CAT_ilist_push(list, i);
	}
}