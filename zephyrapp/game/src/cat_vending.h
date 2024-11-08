#ifndef CAT_VENDING_H
#define CAT_VENDING_H

typedef struct CAT_vending_state
{
	int base;
	int idx;
} CAT_vending_state;
extern CAT_vending_state vending_state;

extern void CAT_render_vending();

#endif