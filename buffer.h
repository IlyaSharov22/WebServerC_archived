#pragma once

#include "scalar.h"

struct buffer {
	u8* data;
	size_t size;
};

void buffer_allocate(struct buffer* buffer, usize size);
void buffer_free(struct buffer* buffer);
void buffer_extend(struct buffer* buffer, usize new_size);