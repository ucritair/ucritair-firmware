#include "cat_alloc.h"

#include "cat_core.h"

void CAT_balloc_init(CAT_bump_allocator* balloc, uint8_t* buffer, size_t capacity)
{
	balloc->buffer = buffer;
	balloc->capacity = capacity;
	balloc->head = 0;
}

uint8_t* CAT_balloc(CAT_bump_allocator* balloc, size_t size)
{
	if(balloc->head + size > balloc->capacity)
		return NULL;
	uint8_t* pointer = &balloc->buffer[balloc->head];
	balloc->head += size;
	return pointer;
}

void CAT_bfree(CAT_bump_allocator* balloc)
{
	balloc->head = 0;
}