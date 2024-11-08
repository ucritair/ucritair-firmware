#include "cat_machine.h"

typedef struct CAT_bag_state
{
	int base;
	int idx;
	CAT_machine_state destination;
} CAT_bag_state;
extern CAT_bag_state bag_state;

void CAT_bag_state_init();