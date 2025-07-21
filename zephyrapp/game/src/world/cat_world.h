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
void CAT_world_get_position(int* x, int* y);
void CAT_world_get_eye(int* x, int* y);


//////////////////////////////////////////////////////////////////////////
// INTERACTION

typedef struct
{
	const char* name;
	uint16_t x, y, w, h;
	int8_t tx, ty;
	const CAT_sprite* sprite;
	void (*proc) ();
} CAT_interactable;

typedef struct
{
	CAT_interactable data[CAT_WORLD_MAX_INTERACTABLE_COUNT];
	int length;
} CAT_interactable_list;

CAT_interactable_list* CAT_world_get_interactables();
int CAT_world_place_interactable(const char* name, const CAT_sprite* sprite, int x, int y, int tx, int ty, void (*proc)());


//////////////////////////////////////////////////////////////////////////
// WORLD

void CAT_MS_world(CAT_machine_signal signal);
void CAT_render_world();

