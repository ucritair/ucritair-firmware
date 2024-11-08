#include "cat_machine.h"

typedef struct CAT_menu_state
{
	const char* titles[6];
	CAT_machine_state states[6];
	int selector;
} CAT_menu_state;
extern CAT_menu_state menu_state;

void CAT_menu_state_init();