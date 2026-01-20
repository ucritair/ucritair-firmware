#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
	uint8_t* buffer;
	size_t capacity;
	size_t head;
} CAT_bump_allocator;

#define CAT_BALLOC_INIT(_buffer, _capacity) (CAT_bump_allocator) \
{ \
	.buffer = _buffer, \
	.capacity = _capacity, \
	.head = 0 \
}

uint8_t* CAT_balloc(CAT_bump_allocator* balloc, size_t size);
void CAT_bfree(CAT_bump_allocator* balloc);
