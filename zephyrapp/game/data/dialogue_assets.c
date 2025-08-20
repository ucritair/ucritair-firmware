#include "dialogue_assets.h"
#include "cat_procs.h"

const CAT_dialogue_node dialogue_reed_default =
{
	.lines = (const char*[])
	{
		"Oh, hello...",
		"I was just grocery shopping.",
		"Have you eaten anything good lately?",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
		{
			.text = "Yes",
			.node = &dialogue_reed_congratulate_food,
			.proc = NULL,
		},
		{
			.text = "Not really",
			.node = &dialogue_reed_admonish_no_food,
			.proc = NULL,
		},
	},
	.edge_count = 2,
};

const CAT_dialogue_node dialogue_reed_house =
{
	.lines = (const char*[])
	{
		"A mysterious cat-shaped house, its door locked.",
		"You think Inpsector Reed lives here.",
	},
	.line_count = 2,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_statue_default =
{
	.lines = (const char*[])
	{
		"Welcome to Wetterstrom, Founded 2025",
		"Return to the apartment?",
	},
	.line_count = 2,
	.edges = (const CAT_dialogue_edge[])
	{
		{
			.text = "Yes",
			.node = NULL,
			.proc = proc_coc_innerworld,
		},
		{
			.text = "No",
			.node = NULL,
			.proc = NULL,
		},
	},
	.edge_count = 2,
};

const CAT_dialogue_node dialogue_statue_chirp =
{
	.lines = (const char*[])
	{
		"\1 Cheep Cheep \1",
	},
	.line_count = 1,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_ellipsis =
{
	.lines = (const char*[])
	{
		"...",
	},
	.line_count = 1,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_reed_admonish_no_food =
{
	.lines = (const char*[])
	{
		"It's important, you know...",
		"You need to eat nutritious food.",
		"Eating is the privilege of the living!",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_reed_congratulate_food =
{
	.lines = (const char*[])
	{
		"Oh, I wonder what it was.",
		"I've been thinking about what to eat all day...",
		"I want... roman gnocchi. Or mashed potatoes.",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_reed_question_fish =
{
	.lines = (const char*[])
	{
		"Hi.",
		"I've been thinking about fish...",
		"Are you partial to any particular fish?",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
		{
			.text = "Mackerel",
			.node = &dialogue_reed_congratulate_fish,
			.proc = NULL,
		},
		{
			.text = "Coelecanth",
			.node = &dialogue_reed_amused_fish,
			.proc = NULL,
		},
		{
			.text = "No",
			.node = &dialogue_admonish_1,
			.proc = NULL,
		},
	},
	.edge_count = 3,
};

const CAT_dialogue_node dialogue_admonish_1 =
{
	.lines = (const char*[])
	{
		"What's wrong with you?",
	},
	.line_count = 1,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_reed_congratulate_fish =
{
	.lines = (const char*[])
	{
		"Oh! That's my favourite too.",
		"Tender white flesh... Oily but not greasy...",
		"You might have good taste.",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_reed_amused_fish =
{
	.lines = (const char*[])
	{
		"What?",
		"Can you taste how rare they are or something?",
	},
	.line_count = 2,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_market_default =
{
	.lines = (const char*[])
	{
		"The lights in the market are dim,",
		"and the glass doors are locked tight.",
		"You recall the time you bought 200 packets of P-cari Sweat drink mix.",
	},
	.line_count = 3,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_arcade_default =
{
	.lines = (const char*[])
	{
		"It looks like the arcade hasn't opened yet.",
		"You could swear you heard an employee practicing one of the drumming games.",
	},
	.line_count = 2,
	.edges = (const CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};


