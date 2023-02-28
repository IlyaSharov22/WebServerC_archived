#pragma once

#include <stdint.h>
#include "includes.h"

void sockets_init();

enum {
	SOCKET_TCP
};

fd_t sockets_new(int protocol);
void sockets_close(fd_t fd);

enum {
	SOCKET_IPV4_ADDRESS
};

struct sockets_address {
	u32 ip;
	u16 port;
};

void sockets_ipv4_init(struct sockets_address* address);
i8 sockets_ipv4_from_string(struct sockets_address* address, const char* ipv4_text);

i8 sockets_bind(fd_t fd, struct sockets_address* address);
i8 sockets_listen(fd_t fd);

fd_t sockets_accept(fd_t server_fd);

i32 sockets_send(fd_t fd, const u8* buffer, usize size);
fd_t sockets_make_server(struct sockets_address* address);