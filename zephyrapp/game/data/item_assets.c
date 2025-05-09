#include "item_assets.h"

#include "cat_item.h"
#include "sprite_assets.h"

CAT_item_table item_table =
{
	.data =
	{
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Disposessed",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "We Are Legion",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Forever War",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Ringworld",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Machine Stops",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Metal Fever",
			.sprite = &book_study_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor = &book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Barley Tea",
			.sprite = &barley_tea_sprite,
			.price = 3,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &barley_tea_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Ebi-Ten Bento",
			.sprite = &bento_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &bento_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MEAT,
				.food_role = CAT_FOOD_ROLE_MAIN,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Bread",
			.sprite = &bread_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &bread_sprite,
				.dv = 2,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_STARCH,
				.food_role = CAT_FOOD_ROLE_STAPLE,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Cigarettes",
			.sprite = &cigarette_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &cigarette_sprite,
				.dv = -1,
				.df = 3,
				.ds = 1,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_VICE,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Coffee",
			.sprite = &coffee_sprite,
			.price = 2,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &coffee_sprite,
				.dv = 1,
				.df = 1,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Green Curry",
			.sprite = &green_curry_sprite,
			.price = 15,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &green_curry_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_VEG,
				.food_role = CAT_FOOD_ROLE_MAIN,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Red Curry",
			.sprite = &red_curry_sprite,
			.price = 15,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &red_curry_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MEAT,
				.food_role = CAT_FOOD_ROLE_MAIN,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Milk",
			.sprite = &milk_sprite,
			.price = 3,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &milk_sprite,
				.dv = 1,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_DAIRY,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Oranges",
			.sprite = &orange_sprite,
			.price = 3,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &orange_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_VEG,
				.food_role = CAT_FOOD_ROLE_SIDE,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Pad Ka Prow",
			.sprite = &padkaprow_sprite,
			.price = 15,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &padkaprow_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MEAT,
				.food_role = CAT_FOOD_ROLE_MAIN,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Persimmons",
			.sprite = &persimmons_sprite,
			.price = 4,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &persimmons_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_VEG,
				.food_role = CAT_FOOD_ROLE_SIDE,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Focus Pill",
			.sprite = &pill_foc_sprite,
			.price = 1,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &pill_foc_sprite,
				.dv = 0,
				.df = 1,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Spirit Pill",
			.sprite = &pill_spi_sprite,
			.price = 1,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &pill_spi_sprite,
				.dv = 0,
				.df = 0,
				.ds = 1,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Vigour Pill",
			.sprite = &pill_vig_sprite,
			.price = 1,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &pill_vig_sprite,
				.dv = 1,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MISC,
				.food_role = CAT_FOOD_ROLE_DRINK,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Raw Meat",
			.sprite = &meat_sprite,
			.price = 6,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &meat_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MEAT,
				.food_role = CAT_FOOD_ROLE_TREAT,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Salad",
			.sprite = &salad_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &salad_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_VEG,
				.food_role = CAT_FOOD_ROLE_SIDE,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Sausage",
			.sprite = &sausage_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &sausage_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_MEAT,
				.food_role = CAT_FOOD_ROLE_MAIN,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Soup",
			.sprite = &soup_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor = &soup_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,

				.food_group = CAT_FOOD_GROUP_VEG,
				.food_role = CAT_FOOD_ROLE_SOUP,
			}
		},
		{
			.type = CAT_ITEM_TYPE_KEY,
			.name = "Mask",
			.sprite = &icon_mask_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_key_sprite,
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Gold Bowl",
			.sprite = &bowl_gold_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Grey Bowl",
			.sprite = &bowl_grey_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Mahogany Bowl",
			.sprite = &bowl_mahogany_sprite,
			.price = 7,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Stone Bowl",
			.sprite = &bowl_stone_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Walnut Bowl",
			.sprite = &bowl_walnut_sprite,
			.price = 7,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Daisy Bush",
			.sprite = &bush_daisy_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Lilac Bush",
			.sprite = &bush_lilac_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Bush",
			.sprite = &bush_plain_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Mahogany Chair",
			.sprite = &chair_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 2},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Plastic Chair",
			.sprite = &chair_plastic_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 2},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Stone Chair",
			.sprite = &chair_stone_sprite,
			.price = 15,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 2},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Chess Set",
			.sprite = &chess_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 2},
				.animate = true,
				.child_dy = -16,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Chicken",
			.sprite = &chicken_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 1},
				.animate = true,
				.child_dy = -6,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Coffee Machine",
			.sprite = &coffeemaker_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 1},
				.animate = true,
				.child_dy = -6,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Blue Crystal",
			.sprite = &crystal_blue_lg_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Green Crystal",
			.sprite = &crystal_green_lg_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Purple Crystal",
			.sprite = &crystal_purple_lg_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Blue Effigy",
			.sprite = &effigy_blue_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {3, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Purple Effigy",
			.sprite = &effigy_purple_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {3, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Sea Effigy",
			.sprite = &effigy_sea_sprite,
			.price = 60,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {3, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Ethereum Farm",
			.sprite = &gpu_sprite,
			.price = 100,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {3, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Classic Fan",
			.sprite = &fan_a_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Modern Fan",
			.sprite = &fan_b_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Smart Flower",
			.sprite = &flower_foc_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Lantern",
			.sprite = &lantern_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Laptop",
			.sprite = &laptop_sprite,
			.price = 80,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {2, 1},
				.animate = true,
				.child_dy = -8,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Naval Mine",
			.sprite = &prop_mine_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Pad Ka Prop",
			.sprite = &padkaprop_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Pixel",
			.sprite = &pixel_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Corn Plant",
			.sprite = &plant_corn_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Daisy Plant",
			.sprite = &plant_daisy_sprite,
			.price = 25,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Lilac Plant",
			.sprite = &plant_lilac_sprite,
			.price = 25,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Maroon Plant",
			.sprite = &plant_maroon_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Plain Plant",
			.sprite = &plant_plain_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Air Purifier",
			.sprite = &purifier_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Happy Flower",
			.sprite = &flower_spi_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Gold Stool",
			.sprite = &stool_gold_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Plastic Stool",
			.sprite = &stool_plastic_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Stone Stool",
			.sprite = &stool_stone_sprite,
			.price = 15,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Wood Stool",
			.sprite = &stool_wood_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Succulent",
			.sprite = &succulent_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Mahogany Table",
			.sprite = &table_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {4, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Plastic Table A",
			.sprite = &table_plastic_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {4, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Plastic Table B",
			.sprite = &table_plastic_alt_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {4, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "S. Mahogany Table",
			.sprite = &table_sm_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "S. Plastic Table A",
			.sprite = &table_sm_plastic_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "S. Plastic Table B",
			.sprite = &table_sm_plastic_alt_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "S. Walnut Table A",
			.sprite = &table_sm_walnut_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "S. Walnut Table B",
			.sprite = &table_sm_walnut_alt_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {2, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Walnut Table",
			.sprite = &table_walnut_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_BOTTOM,
				.shape = {4, 2},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "UV Lamp",
			.sprite = &uv_sprite,
			.price = 50,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Gold Vase",
			.sprite = &vase_gold_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Grey Vase",
			.sprite = &vase_grey_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Mahogany Vase",
			.sprite = &vase_mahogany_sprite,
			.price = 7,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Stone Vase",
			.sprite = &vase_stone_sprite,
			.price = 7,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Walnut Vase",
			.sprite = &vase_walnut_sprite,
			.price = 7,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_TOP,
				.shape = {1, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Healthy Flower",
			.sprite = &flower_vig_sprite,
			.price = 40,
			.text = "",
			.icon = &icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = false,
				.child_dy = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Baseball",
			.sprite = &toy_baseball_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &toy_baseball_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Basketball",
			.sprite = &toy_basketball_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &toy_basketball_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Ducky",
			.sprite = &toy_duck_sprite,
			.price = 30,
			.text = "",
			.icon = &icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &toy_duck_sprite,
				.dv = 0,
				.df = 0,
				.ds = 5,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Golf Ball",
			.sprite = &toy_golf_sprite,
			.price = 5,
			.text = "",
			.icon = &icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &toy_golf_sprite,
				.dv = 0,
				.df = 0,
				.ds = 1,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Laser Pointer",
			.sprite = &laser_pointer_sprite,
			.price = 10,
			.text = "",
			.icon = &icon_item_key_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &laser_marker_sprite,
				.dv = 0,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Puzzle",
			.sprite = &toy_puzzle_sprite,
			.price = 20,
			.text = "",
			.icon = &icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor = &toy_puzzle_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
	},
	.length = 88
};
