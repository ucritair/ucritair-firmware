#include "cat_item.h"
#include "item_assets.h"
#include "sprite_assets.h"

CAT_item_table item_table =
{
.data = 
{
	[14] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Bread",
		.sprite = &bread_sprite,
		.price = 3,
		.text = "A freshly baked loaf of bread. It has a soft crust and a slightly cakey crumb.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &bread_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_STARCH,
		.food_role = CAT_FOOD_ROLE_STAPLE,
	},
	[18] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Coffee",
		.sprite = &coffee_sprite,
		.price = 3,
		.text = "A hot cup of coffee. It's a dark roast with a chocolatey flavour. Some might think it almost tastes burnt.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &coffee_sprite,
		.tool_dv = 1,
		.tool_df = 1,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_DRINK,
	},
	[28] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Milk",
		.sprite = &milk_sprite,
		.price = 3,
		.text = "Fresh milk in a white paper carton. Somehow rich and refreshing all at once.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &milk_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_DAIRY,
		.food_role = CAT_FOOD_ROLE_DRINK,
	},
	[42] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Soup",
		.sprite = &soup_sprite,
		.price = 3,
		.text = "Piping hot soup. Excellent in sickness or in health.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &soup_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SOUP,
	},
	[38] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Salad",
		.sprite = &salad_sprite,
		.price = 3,
		.text = "A crisp fresh salad with just enough dressing. It's even got toasted pumpkin seeds.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &salad_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[40] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Sausage",
		.sprite = &sausage_sprite,
		.price = 3,
		.text = "Sausages grilled on a skewer. The casing is snappy and gives way to a meaty interior.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &sausage_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[20] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Green Curry",
		.sprite = &green_curry_sprite,
		.price = 12,
		.text = "A hot curry that tastes of coconut and green chili.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &green_curry_sprite,
		.tool_dv = 3,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[21] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Red Curry",
		.sprite = &red_curry_sprite,
		.price = 12,
		.text = "A deep red curry with a warming heat behind it.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &red_curry_sprite,
		.tool_dv = 3,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[31] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Pad Ka Prow",
		.sprite = &padkaprow_sprite,
		.price = 12,
		.text = "Perfectly browned ground meat with wisps of basil over rice. It's got everything, even an egg.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &padkaprow_sprite,
		.tool_dv = 3,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[35] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Vigour Seed",
		.sprite = &pill_vig_sprite,
		.price = 3,
		.text = "A bitter pill that will reinvigorate you.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &pill_vig_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_VICE,
	},
	[33] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Focus Seed",
		.sprite = &pill_foc_sprite,
		.price = 3,
		.text = "A bitter pill that will sharpen your focus.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &pill_foc_sprite,
		.tool_dv = 1,
		.tool_df = 1,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_VICE,
	},
	[34] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Spirit Seed",
		.sprite = &pill_spi_sprite,
		.price = 3,
		.text = "A bitter pill that will raise your spirits.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &pill_spi_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 1,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_VICE,
	},
	[17] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Cigarettes",
		.sprite = &cigarette_sprite,
		.price = 0,
		.text = "They're awful for you. Where did you even find these?",
		.can_buy = false,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &cigarette_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 1,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_VICE,
	},
	[0] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "The Disposessed",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "Nothing is yours. It is to use. It is to share. If you will not share it, you cannot use it.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[1] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "A Wizard of Earthsea",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "Go to bed; tired is stupid.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[2] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "The Beauty of Everyday Things",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "There is no greater opportunity for appreciating beauty than through its use in our everyday lives.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[3] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "The Uses of Disorder",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "The desire for purified identity is a state of absolute bondage to the status quo.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[4] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Structure and Interpretation of Computer Programs",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "The programmer must seek both perfection of part and adequacy of collection.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[5] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Kokoro",
		.sprite = &book_study_sprite,
		.price = 7,
		.text = "Now I am going to destroy my heart myself, and pour my blood into your veins.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_BOOK,
		.tool_cursor = &book_static_sprite,
		.tool_dv = 0,
		.tool_df = 3,
		.tool_ds = 0,
	},
	[105] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Ducky",
		.sprite = &toy_duck_sprite,
		.price = 21,
		.text = "A rubber ducky. Squeeze it to see if it makes a noise.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &toy_duck_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 3,
	},
	[103] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Baseball",
		.sprite = &toy_baseball_sprite,
		.price = 7,
		.text = "White leather is pulled perfectly taut over its core. Its red stitching gleams softly.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &toy_baseball_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 1,
	},
	[104] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Basketball",
		.sprite = &toy_basketball_sprite,
		.price = 14,
		.text = "It bounces instantly up from the floor. The smell of fresh rubber.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &toy_basketball_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 2,
	},
	[106] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Golf Ball",
		.sprite = &toy_golf_sprite,
		.price = 7,
		.text = "Just by looking at it you can hear the crack of the club's contact.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &toy_golf_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 1,
	},
	[108] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Puzzle",
		.sprite = &toy_puzzle_sprite,
		.price = 14,
		.text = "If you touch it, you'll start looking for a place to put it.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &toy_puzzle_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 2,
	},
	[45] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Mask",
		.sprite = &icon_mask_sprite,
		.price = 10,
		.text = "This mask can protect you from some, but certainly not all, airborne risks.",
		.can_buy = true,
		.can_sell = false,
	},
	[66] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Defunct Crafting Station",
		.sprite = &prop_crafter_sprite,
		.price = 100,
		.text = "Though dormant now, it is clearly a fine machine. Nothing made so well can be useless for long.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {3, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[96] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "UV Lamp",
		.sprite = &uv_sprite,
		.price = 50,
		.text = "Its ghostly light will banish airborne pathogens.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[80] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Air Purifier",
		.sprite = &purifier_sprite,
		.price = 50,
		.text = "It processes the air around you, leaving it cleaner than before.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[59] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Coffee Machine",
		.sprite = &coffeemaker_sprite,
		.price = 40,
		.text = "It is simple to use and produces serviceable coffee. Its gentle sound colours your mornings.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = -6,
	},
	[67] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Classic Fan",
		.sprite = &fan_a_sprite,
		.price = 20,
		.text = "Not state of the art, but tried and true. A vision of relief in summer.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[68] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Modern Fan",
		.sprite = &fan_b_sprite,
		.price = 30,
		.text = "A rather advanced fan. Popular with those who seek to understand air.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[70] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Lantern",
		.sprite = &lantern_sprite,
		.price = 20,
		.text = "A sturdy but finely wrought lantern. Its light spills over its metalwork.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[71] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Laptop",
		.sprite = &laptop_sprite,
		.price = 80,
		.text = "A surprisingly sturdy laptop. It invites you to fill it with software.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = -8,
	},
	[57] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Chess Set",
		.sprite = &chess_sprite,
		.price = 40,
		.text = "A beautiful chess set with polished pieces. Your critter is smart, but can it really use this?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = -16,
	},
	[87] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Mahogany Table",
		.sprite = &table_mahogany_sprite,
		.price = 20,
		.text = "A sturdy table. Its top looks like it can stand up to anything.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {4, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[95] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Walnut Table",
		.sprite = &table_walnut_sprite,
		.price = 20,
		.text = "A sturdy table. Its top looks like it can stand up to anything.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {4, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[88] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Plastic Table A",
		.sprite = &table_plastic_sprite,
		.price = 20,
		.text = "Perfect for a lawn party or a snack spread.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {4, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[89] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Plastic Table B",
		.sprite = &table_plastic_alt_sprite,
		.price = 20,
		.text = "Its shape is imposing.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {4, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[90] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "S. Mahogany Table",
		.sprite = &table_sm_mahogany_sprite,
		.price = 20,
		.text = "A sturdy little table. It would make for a good nightstand.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[93] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "S. Walnut Table A",
		.sprite = &table_sm_walnut_sprite,
		.price = 30,
		.text = "A sturdy little table. It would make for a good nightstand.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[94] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "S. Walnut Table B",
		.sprite = &table_sm_walnut_alt_sprite,
		.price = 30,
		.text = "A sturdy little table. It would make for a good nightstand.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[91] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "S. Plastic Table A",
		.sprite = &table_sm_plastic_sprite,
		.price = 10,
		.text = "A sturdy little table. It would make for a good nightstand.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[92] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "S. Plastic Table B",
		.sprite = &table_sm_plastic_alt_sprite,
		.price = 10,
		.text = "A sturdy little table. It would make for a good nightstand.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 2},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[54] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Mahogany Chair",
		.sprite = &chair_mahogany_sprite,
		.price = 20,
		.text = "It can support the critter's whole weight. And surprisingly, it's really comfortable.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 2},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[56] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Stone Chair",
		.sprite = &chair_stone_sprite,
		.price = 15,
		.text = "It can support the critter's whole weight. And surprisingly, it's really comfortable.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 2},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[83] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Plastic Stool",
		.sprite = &stool_plastic_sprite,
		.price = 5,
		.text = "Good for standing on if you need to get snacks out of the cabinet.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[85] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Wood Stool",
		.sprite = &stool_wood_sprite,
		.price = 10,
		.text = "Good for standing on if you need to get snacks out of the cabinet.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[84] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Stone Stool",
		.sprite = &stool_stone_sprite,
		.price = 15,
		.text = "Good for standing on if you need to get snacks out of the cabinet.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[82] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Gold Stool",
		.sprite = &stool_gold_sprite,
		.price = 30,
		.text = "Good for standing on if you need to get snacks out of the cabinet. And it's gold.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_BOTTOM,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[49] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Stoneware Bowl",
		.sprite = &bowl_stone_sprite,
		.price = 10,
		.text = "A smooth bowl with decent heft. It will keep your meal standing still.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[46] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Gold Bowl",
		.sprite = &bowl_gold_sprite,
		.price = 20,
		.text = "A smooth bowl with decent heft. It will keep your meal standing still.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[100] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Stoneware Vase",
		.sprite = &vase_stone_sprite,
		.price = 7,
		.text = "It can hold a lot more than flowers. Why not put oil or wax in it?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[97] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Gold Vase",
		.sprite = &vase_gold_sprite,
		.price = 20,
		.text = "It can hold a lot more than flowers. Why not put oil or wax in it?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[98] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Grey Vase",
		.sprite = &vase_grey_sprite,
		.price = 5,
		.text = "It can hold a lot more than flowers. Why not put oil or wax in it?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[99] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Mahogany Vase",
		.sprite = &vase_mahogany_sprite,
		.price = 7,
		.text = "It can hold a lot more than flowers. Why not put oil or wax in it?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[101] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Walnut Vase",
		.sprite = &vase_walnut_sprite,
		.price = 7,
		.text = "It can hold a lot more than flowers. Why not put oil or wax in it?",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[86] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Succulent",
		.sprite = &succulent_sprite,
		.price = 5,
		.text = "Small but lively, this succulent does a surprising amount for the atmosphere of your living space.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[53] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Bush",
		.sprite = &bush_plain_sprite,
		.price = 10,
		.text = "A classic bush. It's quite green.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[51] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Daisy Bush",
		.sprite = &bush_daisy_sprite,
		.price = 30,
		.text = "A floral take on the classic bush. Its white accents are eye-catching.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[52] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Lilac Bush",
		.sprite = &bush_lilac_sprite,
		.price = 30,
		.text = "A floral take on the classic bush. Its purple accents are eye-catching.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[79] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Plain Plant",
		.sprite = &plant_plain_sprite,
		.price = 20,
		.text = "It stands tall and proud for something that cannot live past the edges of a pot.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[78] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Maroon Plant",
		.sprite = &plant_maroon_sprite,
		.price = 20,
		.text = "Look closely at the vivid colour of its trunk. It is subtle but striking.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[77] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Lilac Plant",
		.sprite = &plant_lilac_sprite,
		.price = 25,
		.text = "Its purple flowers are a welcome pop of colour at any time of year.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[76] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Daisy Plant",
		.sprite = &plant_daisy_sprite,
		.price = 25,
		.text = "Its bright white flowers are oddly dazzling. They almost don't belong inside.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[102] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Healthy Flower",
		.sprite = &flower_vig_sprite,
		.price = 40,
		.text = "This flower is a symbol of strength. It blooms full when times are good.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[69] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Smart Flower",
		.sprite = &flower_foc_sprite,
		.price = 40,
		.text = "This flower is a symbol of wisdom. It blooms full when times are good.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[81] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Happy Flower",
		.sprite = &flower_spi_sprite,
		.price = 40,
		.text = "This flower is a symbol of fulfillment. It blooms full when times are good.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[60] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Blue Crystal",
		.sprite = &crystal_blue_lg_sprite,
		.price = 50,
		.text = "Nobody is quite sure of this crystal's rarity. Its value comes purely from its beauty.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[61] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Green Crystal",
		.sprite = &crystal_green_lg_sprite,
		.price = 50,
		.text = "Nobody is quite sure of this crystal's rarity. Its value comes purely from its beauty.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[62] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Purple Crystal",
		.sprite = &crystal_purple_lg_sprite,
		.price = 50,
		.text = "Nobody is quite sure of this crystal's rarity. Its value comes purely from its beauty.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[63] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Blue Effigy",
		.sprite = &effigy_blue_sprite,
		.price = 40,
		.text = "Art wrought by some unknown civilization, not of the past but of the future.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {3, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[64] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Purple Effigy",
		.sprite = &effigy_purple_sprite,
		.price = 40,
		.text = "Art wrought by some unknown civilization, not of the past but of the future.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {3, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[65] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Sea Effigy",
		.sprite = &effigy_sea_sprite,
		.price = 60,
		.text = "Art wrought by some unknown civilization, not of the past but of the future.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {3, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[74] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Pixel",
		.sprite = &pixel_sprite,
		.price = 30,
		.text = "Pixel 63118 of 76800. Rank: Sergeant First Class.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[73] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Pad Ka Prop",
		.sprite = &padkaprop_sprite,
		.price = 30,
		.text = "A feast for the eyes, but nothing more.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[47] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Grey Bowl",
		.sprite = &bowl_grey_sprite,
		.price = 5,
		.text = "A smooth bowl with decent heft. It will keep your meal standing still.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[48] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Mahogany Bowl",
		.sprite = &bowl_mahogany_sprite,
		.price = 7,
		.text = "A smooth bowl with decent heft. It will keep your meal standing still.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[50] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Walnut Bowl",
		.sprite = &bowl_walnut_sprite,
		.price = 7,
		.text = "A smooth bowl with decent heft. It will keep your meal standing still.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[55] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Plastic Chair",
		.sprite = &chair_plastic_sprite,
		.price = 10,
		.text = "It can support the critter's whole weight. And surprisingly, it's really comfortable.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 2},
		.prop_animated = false,
		.prop_child_dy = 0,
	},
	[75] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Corn Plant",
		.sprite = &plant_corn_sprite,
		.price = 30,
		.text = "Though it doesn't yet produce vegetables, it may still be handy to keep around.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[58] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Chicken",
		.sprite = &chicken_sprite,
		.price = 50,
		.text = "An exceptionally round bird. Not food, but a dear friend.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = -6,
	},
	[72] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Naval Mine",
		.sprite = &prop_mine_sprite,
		.price = 50,
		.text = "You disarmed this one yourself. Good work.",
		.can_buy = true,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[107] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Laser Pointer",
		.sprite = &laser_pointer_sprite,
		.price = 21,
		.text = "All CATs love to chase a laser pointer.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_TOY,
		.tool_cursor = &laser_marker_sprite,
		.tool_dv = 0,
		.tool_df = 0,
		.tool_ds = 3,
	},
	[6] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "1-Star Fish",
		.sprite = &fish_tier_1_icon,
		.price = 3,
		.text = "A fish of decent but unremarkable quality.",
		.can_buy = false,
		.can_sell = true,
	},
	[7] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "2-Star Fish",
		.sprite = &fish_tier_2_icon,
		.price = 7,
		.text = "A fish of notable quality.",
		.can_buy = false,
		.can_sell = true,
	},
	[8] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "3-Star Fish",
		.sprite = &fish_tier_3_icon,
		.price = 15,
		.text = "An astounding catch.",
		.can_buy = false,
		.can_sell = true,
	},
	[9] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Bagel",
		.sprite = &bagel_sprite,
		.price = 3,
		.text = "Perfectly chewy with a slightly glossy exterior.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &bagel_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_STARCH,
		.food_role = CAT_FOOD_ROLE_STAPLE,
	},
	[10] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Bananas",
		.sprite = &banana_sprite,
		.price = 3,
		.text = "Nature's perfect dessert. And packed with potassium to boot!",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &banana_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[11] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Barley Tea",
		.sprite = &barley_tea_sprite,
		.price = 3,
		.text = "More delicious than water and more refreshing than coffee.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &barley_tea_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_DRINK,
	},
	[16] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Carrots",
		.sprite = &carrot_sprite,
		.price = 3,
		.text = "The critter has the shape of an animal that would love carrots.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &carrot_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[19] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Congee",
		.sprite = &congee_sprite,
		.price = 3,
		.text = "An strong foundation of rice porridge loaded with flavourful add-ons.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &congee_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_STARCH,
		.food_role = CAT_FOOD_ROLE_STAPLE,
	},
	[22] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Donut",
		.sprite = &donut_sprite,
		.price = 3,
		.text = "Dough fried golden brown, topped with sweet crackling icing. Cake or classic, take your pick.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &donut_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_STARCH,
		.food_role = CAT_FOOD_ROLE_TREAT,
	},
	[24] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Egg",
		.sprite = &egg_sprite,
		.price = 3,
		.text = "An egg.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &egg_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_DAIRY,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[26] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Green Tea",
		.sprite = &green_tea_sprite,
		.price = 3,
		.text = "It's somehow always the right temperature. Perfectly bitter.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &green_tea_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_DRINK,
	},
	[30] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Oranges",
		.sprite = &orange_sprite,
		.price = 3,
		.text = "The smell of citrus wafts off of them. You can tell they are sweet just at a glance.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &orange_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[32] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Persimmons",
		.sprite = &persimmons_sprite,
		.price = 3,
		.text = "Just ripe enough to eat, and sweet as candy. Don't eat too many or you'll get a bezoar.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &persimmons_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_SIDE,
	},
	[41] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Soda",
		.sprite = &soda_sprite,
		.price = 3,
		.text = "Sugary and fizzy and strongly flavoured. Drink it cold on a hot day.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &soda_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_TREAT,
	},
	[43] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Strange Meal",
		.sprite = &strange_meal_sprite,
		.price = 3,
		.text = "What could this possibly be? Do you think it tastes good?",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &strange_meal_sprite,
		.tool_dv = 1,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MISC,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[13] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Ebi-Ten Bento",
		.sprite = &bento_sprite,
		.price = 7,
		.text = "Crispy fried shrimp with a moderate helping of rice and a variety of sides.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &bento_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[15] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Burger",
		.sprite = &burger_sprite,
		.price = 7,
		.text = "It's as classic as can be. The toppings and the bun don't outshine the patty itself.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &burger_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[23] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Pork Dumplings",
		.sprite = &dumplings_sprite,
		.price = 7,
		.text = "Steamed dumplings filled with minced pork and aromatic vegetables. The skin's texture is ideal.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &dumplings_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[25] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Beef Empanadas",
		.sprite = &empanadas_sprite,
		.price = 7,
		.text = "Toasty flaky dough covering a richly spiced meat filling. It's hard to stop at a couple.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &empanadas_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[27] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Hotdog",
		.sprite = &hotdog_sprite,
		.price = 7,
		.text = "It's something you might get at a backyard BBQ. It's got the right amount of char.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &hotdog_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[36] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Chashu Ramen",
		.sprite = &ramen_sprite,
		.price = 7,
		.text = "Hot rich broth and just barely toothsome noodles. You look forward to it after a long night.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &ramen_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[37] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Raw Meat",
		.sprite = &meat_sprite,
		.price = 7,
		.text = "It's good meat, no more and no less. Don't forget that your critter is an animal.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &meat_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_TREAT,
	},
	[39] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Salmon",
		.sprite = &salmon_sprite,
		.price = 7,
		.text = "A beautiful salmon. Its eyes are bright and clear and its scales glitter.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &salmon_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[44] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Tuna",
		.sprite = &tuna_sprite,
		.price = 7,
		.text = "Catafalqued king of my own ocean.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &tuna_sprite,
		.tool_dv = 2,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[12] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Beef Noodles",
		.sprite = &beef_noodle_sprite,
		.price = 12,
		.text = "Chewy noodles loaded with tender beef. The smell is divine.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &beef_noodle_sprite,
		.tool_dv = 3,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_MEAT,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[29] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_TOOL,
		.name = "Nasi Lemak",
		.sprite = &nasi_lemak_sprite,
		.price = 12,
		.text = "The smell of coconut and pandan entices you to have a bite of rice loaded with sides.",
		.can_buy = true,
		.can_sell = false,
		
		.tool_type = CAT_TOOL_TYPE_FOOD,
		.tool_cursor = &nasi_lemak_sprite,
		.tool_dv = 3,
		.tool_df = 0,
		.tool_ds = 0,
		
		.food_group = CAT_FOOD_GROUP_VEG,
		.food_role = CAT_FOOD_ROLE_MAIN,
	},
	[109] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Coin",
		.sprite = &coin_static_sprite,
		.price = 0,
		.text = "A gleaming golden coin. The possibilities for its use are endless.",
		.can_buy = false,
		.can_sell = false,
	},
	[110] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Orange Portal",
		.sprite = &portal_orange_sprite,
		.price = 0,
		.text = "If you enter this one...",
		.can_buy = false,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[111] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Blue Portal",
		.sprite = &portal_blue_sprite,
		.price = 0,
		.text = "...you'll come out the other side!",
		.can_buy = false,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[112] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Xen Crystal",
		.sprite = &xen_crystal_sprite,
		.price = 0,
		.text = "Its label says GG-3883. Can't hurt to keep it around for now.",
		.can_buy = false,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_TOP,
		.prop_shape = {1, 1},
		.prop_animated = false,
		.prop_child_dy = -3,
	},
	[113] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_PROP,
		.name = "Hoopy the Hoop",
		.sprite = &prop_hoopy_sprite,
		.price = 0,
		.text = "Does this one look like glados_debris_05 or glados_gib_10 to you?",
		.can_buy = false,
		.can_sell = false,
		
		.prop_type = CAT_PROP_TYPE_DEFAULT,
		.prop_shape = {2, 1},
		.prop_animated = true,
		.prop_child_dy = 0,
	},
	[114] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Migration Mark",
		.sprite = &cursor_flip_sprite,
		.price = 0,
		.text = "Commemorates a great leap forward.",
		.can_buy = false,
		.can_sell = false,
	},
	[115] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Oblivion Mark",
		.sprite = &cursor_remove_sprite,
		.price = 0,
		.text = "Commemorates starting over fresh.",
		.can_buy = false,
		.can_sell = false,
	},
	[116] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Extension Mark",
		.sprite = &cursor_add_sprite,
		.price = 0,
		.text = "Commemorates incremental progress.",
		.can_buy = false,
		.can_sell = false,
	},
	[117] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Wood",
		.sprite = &ingr_wood_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[118] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Stone",
		.sprite = &ingr_stone_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[119] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Raw Fibres",
		.sprite = &ingr_fibre_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[120] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Clay",
		.sprite = &ingr_clay_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[121] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Seeds",
		.sprite = &ingr_seeds_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[122] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Oil",
		.sprite = &ingr_oil_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[123] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Ink",
		.sprite = &ingr_ink_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[124] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Water",
		.sprite = &ingr_water_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[125] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Time",
		.sprite = &ingr_time_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[126] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Aether",
		.sprite = &ingr_aether_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[127] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Miasma",
		.sprite = &ingr_miasma_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[128] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Bone",
		.sprite = &ingr_bone_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[129] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Ceramic",
		.sprite = &ingr_ceramic_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[130] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Charcoal",
		.sprite = &ingr_charcoal_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[131] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Cloth",
		.sprite = &ingr_cloth_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[132] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Glass",
		.sprite = &ingr_glass_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[133] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Leather",
		.sprite = &ingr_leather_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[134] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Paper",
		.sprite = &ingr_paper_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[135] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Salt",
		.sprite = &ingr_salt_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[136] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Sugar",
		.sprite = &ingr_sugar_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
	[137] = (const CAT_item)
	{
		.type = CAT_ITEM_TYPE_KEY,
		.name = "Sand",
		.sprite = &ingr_sand_sprite,
		.price = 0,
		.text = "",
		.can_buy = false,
		.can_sell = false,
	},
},
.length = 138,
};
