#pragma once

#include <stdint.h>
#include "cat_core.h"
#include "cat_machine.h"
#include "cat_render.h"

//////////////////////////////////////////////////////////////////////////
// CONSTANTS

#define CAT_WORLD_X 0
#define CAT_WORLD_Y 0
#define CAT_WORLD_W CAT_LCD_SCREEN_W
#define CAT_WORLD_H CAT_LCD_SCREEN_H
#define CAT_WORLD_MAX_X (CAT_WORLD_X + CAT_WORLD_W-1)
#define CAT_WORLD_MAX_Y (CAT_WORLD_Y + CAT_WORLD_H-1)
#define CAT_WORLD_CENTER_X (CAT_WORLD_X + CAT_WORLD_W/2)
#define CAT_WORLD_CENTER_Y (CAT_WORLD_Y + CAT_WORLD_H/2)

#define CAT_WORLD_MAX_INTERACTABLE_COUNT 32


//////////////////////////////////////////////////////////////////////////
// MOVEMENT

void CAT_world_move_to(int x, int y);
void CAT_world_move_by(int dx, int dy);


//////////////////////////////////////////////////////////////////////////
// INTERACTION

typedef struct
{
	uint16_t x, y, w, h;
	const CAT_sprite* sprite;
	void (*proc) (void);
} CAT_interactable;

typedef struct
{
	CAT_interactable data[CAT_WORLD_MAX_INTERACTABLE_COUNT];
	int length;
} CAT_interactable_list;

CAT_interactable_list* CAT_world_get_interactables();
int CAT_world_place_interactable(CAT_interactable interactable);


//////////////////////////////////////////////////////////////////////////
// WORLD

void CAT_MS_world(CAT_machine_signal signal);
void CAT_render_world();

