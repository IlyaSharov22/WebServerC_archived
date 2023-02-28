#include <stdlib.h>
#include "buffer.h"

void buffer_free(struct buffer* buffer) {
	if (buffer->data) {
		free(buffer->data);
		buffer->data = NULL;
		buffer->size = 0;
	}
}

void buffer_extend(struct buffer* buffer, usize new_size) {
	buffer_free(buffer);
	buffer_allocate(buffer, new_size);
}

void buffer_allocate(struct buffer* buffer, usize size) {
	if (buffer->data) {
		buffer_free(buffer);
	}

	buffer->data = (u8*)malloc(sizeof(u8) * size);
	buffer->size = size;
}