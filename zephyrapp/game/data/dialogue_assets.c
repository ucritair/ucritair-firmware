#include "dialogue_assets.h"

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
			.type = CAT_DIALOGUE_EDGE_TYPE_NODE,
			.node = &dialogue_test_b,
		},
		{
			.text = "No",
			.type = CAT_DIALOGUE_EDGE_TYPE_PROC,
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
	},
	.edge_count = 0,
};


