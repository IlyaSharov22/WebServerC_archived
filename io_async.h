#pragma once

#include "includes.h"
#include "event.h"

typedef void(*async_completion_handler_t)(struct asyncio*, usize, void*);

enum {
	IO_OPERATION_INACTIVE,
	IO_OPERATION_PENDING
};

enum {
	IO_ERROR_OK,
	IO_ERROR_BUFFER_LESS_OR_NULL,
	IO_ERROR_SYSTEM_ERROR
};

struct async_io {
	fd_t fd;
	event_key_t event_key;
	event_loop_t* event_loop;
	async_completion_handler_t async_cb;
	OVERLAPPED overlapped;
	struct buffer transmission_buffer;
	u8 op_state;
};

void async_io_init(struct async_io* io, fd_t fd, event_loop_t* loop);
void async_io_alloc_buffer(struct async_io* io, usize buffer_size);
u8* async_io_buffer_data(struct async_io* io);
u8 async_io_net_read(struct async_io* io, usize length, async_completion_handler_t read_cb);
void async_io_destroy(struct async_io* io);