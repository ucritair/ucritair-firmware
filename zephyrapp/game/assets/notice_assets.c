#include "notice_assets.h"

const CAT_notice CAT_notice_list[] =
{
	(const CAT_notice) {
		.string = "A fresh breeze blows",
		.tags = CAT_CONTENT_TAG_AQ_GOOD,
	},
	(const CAT_notice) {
		.string = "The critter breathes gently",
		.tags = CAT_CONTENT_TAG_AQ_GOOD,
	},
	(const CAT_notice) {
		.string = "There is the faintest scent of flowers on the air",
		.tags = CAT_CONTENT_TAG_AQ_GOOD,
	},
	(const CAT_notice) {
		.string = "The critter look dazed",
		.tags = CAT_CONTENT_TAG_CO2_BAD,
	},
	(const CAT_notice) {
		.string = "The critter takes an uneasy breath",
		.tags = CAT_CONTENT_TAG_CO2_BAD,
	},
	(const CAT_notice) {
		.string = "A stagnant atmosphere hangs",
		.tags = CAT_CONTENT_TAG_CO2_BAD,
	},
	(const CAT_notice) {
		.string = "A thick haze hangs",
		.tags = CAT_CONTENT_TAG_PM_BAD,
	},
	(const CAT_notice) {
		.string = "The critter wheezes",
		.tags = CAT_CONTENT_TAG_PM_BAD,
	},
	(const CAT_notice) {
		.string = "The critter's eyes water",
		.tags = CAT_CONTENT_TAG_PM_BAD,
	},
	(const CAT_notice) {
		.string = "A gaseous smell wafts by",
		.tags = CAT_CONTENT_TAG_NOX_VOC_BAD,
	},
	(const CAT_notice) {
		.string = "Fumes, perhaps from burning fuel",
		.tags = CAT_CONTENT_TAG_NOX_VOC_BAD,
	},
	(const CAT_notice) {
		.string = "The creature sniffs at something chemical",
		.tags = CAT_CONTENT_TAG_NOX_VOC_BAD,
	},
	(const CAT_notice) {
		.string = "The critter pants rapidly",
		.tags = CAT_CONTENT_TAG_TEMP_BAD,
	},
	(const CAT_notice) {
		.string = "A wave of intense heat rolls by",
		.tags = CAT_CONTENT_TAG_TEMP_BAD,
	},
	(const CAT_notice) {
		.string = "An egg sizzles on pavement",
		.tags = CAT_CONTENT_TAG_TEMP_BAD,
	},
	(const CAT_notice) {
		.string = "Mosquitos swarm the muggy air",
		.tags = CAT_CONTENT_TAG_RH_BAD,
	},
	(const CAT_notice) {
		.string = "The walls almost feel damp",
		.tags = CAT_CONTENT_TAG_RH_BAD,
	},
	(const CAT_notice) {
		.string = "The critter wishes it could sweat",
		.tags = CAT_CONTENT_TAG_RH_BAD,
	},
	(const CAT_notice) {
		.string = "The critter flexes, energetic and ready",
		.tags = CAT_CONTENT_TAG_STATS_GOOD,
	},
	(const CAT_notice) {
		.string = "The critter is thinking about something intently",
		.tags = CAT_CONTENT_TAG_STATS_GOOD,
	},
	(const CAT_notice) {
		.string = "The critter dances playfully",
		.tags = CAT_CONTENT_TAG_STATS_GOOD,
	},
	(const CAT_notice) {
		.string = "The critter's shoulders slump with exhaustion",
		.tags = CAT_CONTENT_TAG_STATS_BAD,
	},
	(const CAT_notice) {
		.string = "The critter is distractedly fidgeting",
		.tags = CAT_CONTENT_TAG_STATS_BAD,
	},
	(const CAT_notice) {
		.string = "The critter sports a sour look",
		.tags = CAT_CONTENT_TAG_STATS_BAD,
	},
	(const CAT_notice) {
		.string = "Crickets sing a droning song",
		.tags = CAT_CONTENT_TAG_SPRING,
	},
	(const CAT_notice) {
		.string = "A chickadee's call tumbles over itself",
		.tags = CAT_CONTENT_TAG_SPRING,
	},
	(const CAT_notice) {
		.string = "A bluejay's cry pierces the cool air",
		.tags = CAT_CONTENT_TAG_SPRING,
	},
	(const CAT_notice) {
		.string = "The calling of cicadas penetrates the stones",
		.tags = CAT_CONTENT_TAG_SUMMER,
	},
	(const CAT_notice) {
		.string = "A warm wind cushions you",
		.tags = CAT_CONTENT_TAG_SUMMER,
	},
	(const CAT_notice) {
		.string = "Somewhere, waves crash on sand",
		.tags = CAT_CONTENT_TAG_SUMMER,
	},
	(const CAT_notice) {
		.string = "Leaves rustle somewhere nearby",
		.tags = CAT_CONTENT_TAG_AUTUMN,
	},
	(const CAT_notice) {
		.string = "A cool and dry wind blows a twig over stones",
		.tags = CAT_CONTENT_TAG_AUTUMN,
	},
	(const CAT_notice) {
		.string = "An owl looses a mournful call",
		.tags = CAT_CONTENT_TAG_AUTUMN,
	},
	(const CAT_notice) {
		.string = "Snow falls, blanketing the world",
		.tags = CAT_CONTENT_TAG_WINTER,
	},
	(const CAT_notice) {
		.string = "What light remains glitters in the ice",
		.tags = CAT_CONTENT_TAG_WINTER,
	},
	(const CAT_notice) {
		.string = "The smell of spices warms you",
		.tags = CAT_CONTENT_TAG_WINTER,
	},
	(const CAT_notice) {
		.string = "Sunlight beams in from the world",
		.tags = CAT_CONTENT_TAG_MORNING,
	},
	(const CAT_notice) {
		.string = "The waking city calls out to you",
		.tags = CAT_CONTENT_TAG_MORNING,
	},
	(const CAT_notice) {
		.string = "A train speeds its charges to work",
		.tags = CAT_CONTENT_TAG_MORNING,
	},
	(const CAT_notice) {
		.string = "A rooster crows proudly",
		.tags = CAT_CONTENT_TAG_MORNING,
	},
	(const CAT_notice) {
		.string = "The critter yawns prematurely",
		.tags = CAT_CONTENT_TAG_DAY,
	},
	(const CAT_notice) {
		.string = "A shop door jingles as it opens",
		.tags = CAT_CONTENT_TAG_DAY,
	},
	(const CAT_notice) {
		.string = "A freight train's great body slithers past",
		.tags = CAT_CONTENT_TAG_DAY,
	},
	(const CAT_notice) {
		.string = "A man calls out for a taxi",
		.tags = CAT_CONTENT_TAG_DAY,
	},
	(const CAT_notice) {
		.string = "Jazz wafts in from a dimly lit bar",
		.tags = CAT_CONTENT_TAG_NIGHT,
	},
	(const CAT_notice) {
		.string = "The sound of laughter and clinking glass is heard",
		.tags = CAT_CONTENT_TAG_NIGHT,
	},
	(const CAT_notice) {
		.string = "A cat meows just once at the moon",
		.tags = CAT_CONTENT_TAG_NIGHT,
	},
	(const CAT_notice) {
		.string = "The last train speeds home",
		.tags = CAT_CONTENT_TAG_NIGHT,
	},
	(const CAT_notice) {
		.string = "A grave egg feels no pain",
		.tags = CAT_CONTENT_TAG_PET_DEAD,
	},
	(const CAT_notice) {
		.string = "A distant bell sounds",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "An evil star is shining",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "A salamander scurries into flame to be destroyed",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "The dead dream that they live",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "The vending machine hums softly",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "The arcade lets loose a single beep",
		.tags = CAT_CONTENT_TAG_NONE,
	},
	(const CAT_notice) {
		.string = "A smaller critter scurries across the floor",
		.tags = CAT_CONTENT_TAG_NONE,
	},
};
