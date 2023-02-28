#include "io_async.h"

void async_io_init(struct async_io* io, fd_t fd, event_loop_t* loop) {
	io->async_cb = NULL;
	io->event_loop = loop;
	io->fd = fd;
	io->event_key = event_add(fd, loop);
	io->op_state = IO_OPERATION_INACTIVE;

	event_set_user_data(io->event_key, io);

	memset(&io->transmission_buffer, 0, sizeof(struct buffer));
}

void async_io_alloc_buffer(struct async_io* io, usize buffer_size) {
	buffer_allocate(&io->transmission_buffer, buffer_size);
}

u8* async_io_buffer_data(struct async_io* io) {
	return io->transmission_buffer.data;
}

static void event_read_callback(event_loop_t* loop, usize transferred, void* user_data) {
	struct async_io* io = (struct async_io*)user_data;

	if (io->op_state = IO_OPERATION_PENDING) {
		io->op_state = IO_OPERATION_INACTIVE;

		async_completion_handler_t handler = io->async_cb;
		io->async_cb = NULL;

		handler(io, transferred, user_data);
	}
}

u8 async_io_net_read(struct async_io* io, usize length, async_completion_handler_t read_cb) {
	if (io->transmission_buffer.size < length) return IO_ERROR_BUFFER_LESS_OR_NULL;

	memset(&io->overlapped, 0, sizeof(OVERLAPPED));

	DWORD flags = 0;

	WSABUF buffers;
	buffers.buf = io->transmission_buffer.data;
	buffers.len = (ULONG)io->transmission_buffer.size;

	io->async_cb = read_cb;

	event_submit(io->event_key, event_read_callback);
	int result = WSARecv((SOCKET)io->fd, &buffers, 1, NULL, &flags, &io->overlapped, NULL);

	if (result != 0) {
		int error = WSAGetLastError();

		if (error != WSA_IO_PENDING) {
			return IO_ERROR_SYSTEM_ERROR;
		}
	}

	io->op_state = IO_OPERATION_PENDING;

	return IO_ERROR_OK;
}

void async_io_destroy(struct async_io* io) {
	event_close(io->event_loop, io->event_key);
	CloseHandle(io->fd);
	buffer_free(&io->transmission_buffer);
}