#include "dialogue_assets.h"
#include "cat_procs.h"

const CAT_dialogue_node dialogue_reed_default =
{
	.lines = (const char*[])
	{
		"Hey...",
		"Eaten anything good lately?",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
	{
		{
			.text = "...",
			.node = &dialogue_ellipsis,
			.proc = NULL,
		},
	},
	.edge_count = 1,
};

const CAT_dialogue_node dialogue_reed_house =
{
	.lines = (const char*[])
	{
		"[It's Inspector Reed's house]",
		"[The doors are locked tight]",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};

const CAT_dialogue_node dialogue_statue_default =
{
	.lines = (const char*[])
	{
		"Welcome to Wetterstrom, Founded 2025",
		"[Return to the apartment?]",
	},
	.line_count = 2,
	.edges = (CAT_dialogue_edge[])
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
	.edges = (CAT_dialogue_edge[])
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
	.edges = (CAT_dialogue_edge[])
	{
	},
	.edge_count = 0,
};


