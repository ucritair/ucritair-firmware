#include "dialogue_assets.h"
#include "cat_dialogue_procs.h"

const CAT_dialogue_node dialogue_test_a =
{
	.lines = (const char*[])
	{
		"Hello, player!",
		"It's time I asked you a question...",
		"Do you want to keep talking to me?",
	},
	.line_count = 3,
	.edges = (CAT_dialogue_edge[])
	{
		{
			.text = "Yes",
			.node = &dialogue_test_b,
			.proc = NULL,
		},
		{
			.text = "No",
			.node = NULL,
			.proc = NULL,
		},
	},
	.edge_count = 2,
};

const CAT_dialogue_node dialogue_test_b =
{
	.lines = (const char*[])
	{
		"Well, that's sweet of you.",
		"I don't have much left to say though...",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
	{
		{
			.text = "Okay",
			.node = &dialogue_test_c,
			.proc = NULL,
		},
	},
	.edge_count = 1,
};

const CAT_dialogue_node dialogue_test_c =
{
	.lines = (const char*[])
	{
		"Okay then, how about this:",
		"Do you want a carrot?",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
	{
		{
			.text = "Yes",
			.node = NULL,
			.proc = dialogue_proc_give_carrot,
		},
		{
			.text = "What?",
			.node = &dialogue_test_d,
			.proc = NULL,
		},
		{
			.text = "No",
			.node = NULL,
			.proc = NULL,
		},
	},
	.edge_count = 3,
};

const CAT_dialogue_node dialogue_test_d =
{
	.lines = (const char*[])
	{
		"A carrot. It's a type of vegetable.",
		"Orange, green on top, sweet and crunchy.",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
	{
		{
			.text = "I see",
			.node = &dialogue_test_c,
			.proc = NULL,
		},
		{
			.text = "Nevermind",
			.node = NULL,
			.proc = NULL,
		},
	},
	.edge_count = 2,
};


