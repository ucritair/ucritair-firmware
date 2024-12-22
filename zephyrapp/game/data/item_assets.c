#include "item_assets.h"

#include "cat_item.h"
#include "sprite_assets.h"

CAT_item_table item_table =
{
	.data =
	{
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Bread",
			.sprite_id = bread_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = bread_sprite,
				.dv = 2,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Coffee",
			.sprite_id = coffee_sprite,
			.price = 2,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = coffee_sprite,
				.dv = 1,
				.df = 1,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Milk",
			.sprite_id = milk_sprite,
			.price = 3,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = milk_sprite,
				.dv = 1,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Soup",
			.sprite_id = soup_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = soup_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Salad",
			.sprite_id = salad_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = salad_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Sausage",
			.sprite_id = sausage_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = sausage_sprite,
				.dv = 3,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Green Curry",
			.sprite_id = green_curry_sprite,
			.price = 15,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = green_curry_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Red Curry",
			.sprite_id = red_curry_sprite,
			.price = 15,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = red_curry_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Pad Ka Prow",
			.sprite_id = padkaprow_sprite,
			.price = 15,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = padkaprow_sprite,
				.dv = 5,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Vigour Pill",
			.sprite_id = pill_vig_sprite,
			.price = 1,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = pill_vig_sprite,
				.dv = 1,
				.df = 0,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Focus Pill",
			.sprite_id = pill_foc_sprite,
			.price = 1,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = pill_foc_sprite,
				.dv = 0,
				.df = 1,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Spirit Pill",
			.sprite_id = pill_spi_sprite,
			.price = 1,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = pill_spi_sprite,
				.dv = 0,
				.df = 0,
				.ds = 1,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Cigarettes",
			.sprite_id = cigarette_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_food_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_FOOD,
				.cursor_id = cigarette_sprite,
				.dv = -1,
				.df = 3,
				.ds = 1,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Disposessed",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "We Are Legion",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Forever War",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Ringworld",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "The Machine Stops",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Metal Fever",
			.sprite_id = book_study_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_book_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_BOOK,
				.cursor_id = book_static_sprite,
				.dv = 0,
				.df = 3,
				.ds = 0,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Ducky",
			.sprite_id = toy_duck_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor_id = toy_duck_sprite,
				.dv = 0,
				.df = 0,
				.ds = 5,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Baseball",
			.sprite_id = toy_baseball_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor_id = toy_baseball_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Basketball",
			.sprite_id = toy_basketball_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor_id = toy_basketball_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Golf Ball",
			.sprite_id = toy_golf_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor_id = toy_golf_sprite,
				.dv = 0,
				.df = 0,
				.ds = 1,
			}
		},
		{
			.type = CAT_ITEM_TYPE_TOOL,
			.name = "Puzzle",
			.sprite_id = toy_puzzle_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_toy_sprite,
			.data.tool_data =
			{
				.type = CAT_TOOL_TYPE_TOY,
				.cursor_id = toy_puzzle_sprite,
				.dv = 0,
				.df = 0,
				.ds = 3,
			}
		},
		{
			.type = CAT_ITEM_TYPE_KEY,
			.name = "Mask",
			.sprite_id = icon_mask_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_key_sprite,
		},
		{
			.type = CAT_ITEM_TYPE_PROP,
			.name = "Ethereum Farm",
			.sprite_id = gpu_sprite,
			.price = 100,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "UV Lamp",
			.sprite_id = uv_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Air Purifier",
			.sprite_id = purifier_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Coffee Machine",
			.sprite_id = coffeemaker_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Classic Fan",
			.sprite_id = fan_a_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = fan_b_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Lantern",
			.sprite_id = lantern_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = laptop_sprite,
			.price = 80,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Chess Set",
			.sprite_id = chess_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Mahogany Table",
			.sprite_id = table_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Walnut Table",
			.sprite_id = table_walnut_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_plastic_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_plastic_alt_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_sm_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_sm_walnut_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_sm_walnut_alt_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_sm_plastic_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = table_sm_plastic_alt_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Mahogany Chair",
			.sprite_id = chair_mahogany_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = chair_stone_sprite,
			.price = 15,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Plastic Stool",
			.sprite_id = stool_plastic_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = stool_wood_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = stool_stone_sprite,
			.price = 15,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Gold Stool",
			.sprite_id = stool_gold_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Stone Bowl",
			.sprite_id = bowl_stone_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Gold Bowl",
			.sprite_id = bowl_gold_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = vase_stone_sprite,
			.price = 7,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = vase_gold_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = vase_grey_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = vase_mahogany_sprite,
			.price = 7,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = vase_walnut_sprite,
			.price = 7,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Succulent",
			.sprite_id = succulent_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Bush",
			.sprite_id = bush_plain_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Daisy Bush",
			.sprite_id = bush_daisy_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = bush_lilac_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = plant_plain_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = plant_maroon_sprite,
			.price = 20,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = plant_lilac_sprite,
			.price = 25,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = plant_daisy_sprite,
			.price = 25,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Healthy Flower",
			.sprite_id = flower_vig_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Smart Flower",
			.sprite_id = flower_foc_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Happy Flower",
			.sprite_id = flower_spi_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Blue Crystal",
			.sprite_id = crystal_blue_lg_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = crystal_green_lg_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = crystal_purple_lg_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = effigy_blue_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = effigy_purple_sprite,
			.price = 40,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = effigy_sea_sprite,
			.price = 60,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Pixel",
			.sprite_id = pixel_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Pad Ka Prop",
			.sprite_id = padkaprop_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Grey Bowl",
			.sprite_id = bowl_grey_sprite,
			.price = 5,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = bowl_mahogany_sprite,
			.price = 7,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.sprite_id = bowl_walnut_sprite,
			.price = 7,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Plastic Chair",
			.sprite_id = chair_plastic_sprite,
			.price = 10,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Corn Plant",
			.sprite_id = plant_corn_sprite,
			.price = 30,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Chicken",
			.sprite_id = chicken_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
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
			.name = "Naval Mine",
			.sprite_id = prop_mine_sprite,
			.price = 50,
			.text = "",
			.icon_id = icon_item_prop_sprite,
			.data.prop_data =
			{
				.type = CAT_PROP_TYPE_DEFAULT,
				.shape = {2, 1},
				.animate = true,
				.child_dy = 0,
			}
		},
	},
	.length = 82
};
