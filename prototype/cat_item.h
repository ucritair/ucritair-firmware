#ifndef CAT_ITEM_H
#define CAT_ITEM_H

typedef enum CAT_item_type
{
	CAT_ITEM_TYPE_PROP,
	CAT_ITEM_TYPE_FOOD
} CAT_item_type;

typedef struct CAT_item
{
	CAT_item_type type;

	const char* name;
	CAT_anim* anim;

	union
	{
		struct
		{
			int tile_width;
			int tile_height;
		} prop_data;

		struct
		{
			float delta_vigor;
			float delta_focus;
			float delta_soul;
		} food_data;
	} data;
} CAT_item;

void CAT_item_init(CAT_item* item, CAT_item_type type, const char* name, CAT_anim* anim)
{
	item->type = type;
	item->name = name;
	item->anim = anim;
}

#endif
