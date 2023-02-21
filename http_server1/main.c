#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "sockets.h"

#define DEBUG_WSA_ERROR() do { int error = WSAGetLastError(); if (error != 0) DEBUG_PRINT_WSA_ERROR(error); } while (0);
#define DEBUG_PRINT_WSA_ERROR(error) printf("WSAGetLastError(): %d on the line %d in %s\n", error, __LINE__, __FILE__)

static const char* http_response() {
	return
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Length: 11\r\n"
		"\r\n"
		"it's works!";
}

int main() {
	sockets_init();

	fd_t fd = sockets_new(SOCKET_TCP);

	struct sockets_address bind_address = { 0 };

	sockets_ipv4_from_string(&bind_address, "127.0.0.1");
	bind_address.port = 80;

	sockets_bind(fd, &bind_address);

	DEBUG_WSA_ERROR();

	sockets_listen(fd);

	fd_t client_fd = sockets_accept(fd);

	const char* client_response = http_response();

	sockets_send(client_fd, (const u8*)client_response, strlen(client_response));

	Sleep(10); // для теста, иначе получаются баги

	sockets_close(client_fd);

	sockets_close(fd);

	sockets_cleanup();
}