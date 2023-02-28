#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sockets.h"
#include "io_async.h"

#define DEBUG_WSA_ERROR() do { int error = WSAGetLastError(); if (error != 0) DEBUG_PRINT_WSA_ERROR(error); } while (0)
#define DEBUG_PRINT_WSA_ERROR(error) printf("WSAGetLastError(): %d on the line %d in %s\n", error, __LINE__, __FILE__)

event_loop_t* g_loop = NULL;

static void read_callback(struct async_io* loop, usize transferred, void* user_data) {
	printf("% " PRIu64 " bytes read:\n\n", transferred);
	
	struct buffer* buffer = (struct buffer*)&loop->transmission_buffer;

	buffer->data[transferred] = '\0';
	puts(buffer->data);

	async_io_destroy(loop);
	event_loop_stop(g_loop);
}

int main() {
	sockets_init();

	g_loop = event_loop_new();

	struct sockets_address server_address;
	sockets_ipv4_from_string(&server_address, "127.0.0.1");
	server_address.port = 80;

	fd_t server = sockets_make_server(&server_address);

	if (!server) {
		DEBUG_WSA_ERROR();
		return 0;
	}

	fd_t new_client = sockets_accept(server);

	struct async_io io;

	async_io_init(&io, new_client, g_loop);

	async_io_alloc_buffer(&io, 512);
	async_io_net_read(&io, 512, read_callback);

	event_loop_run(g_loop);

	event_loop_free(g_loop);
}