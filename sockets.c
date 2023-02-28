#include "sockets.h"
#include <stdio.h>
#include <assert.h>

#define NULL_LITERAL '\0'
#define ZERO_LITERAL '0'
#define DOT_LITERAL '.'

void sockets_init() {
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
}

fd_t sockets_new(int protocol) {
	fd_t fd = 0;
	switch (protocol) {
	case SOCKET_TCP:
		fd = (fd_t)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		return fd == (fd_t)INVALID_SOCKET ? 0 : fd;
	default:
		return 0;
	}
}

void sockets_close(fd_t fd) {
	closesocket((SOCKET)fd);
}

static i8 parse_ipv4_address(const char* address, u8 octets[4]) {
	memset(octets, 0, 4);

	unsigned octet_index = 0;

	while (*address != NULL_LITERAL && octet_index < 4) {

		if (*address == DOT_LITERAL) {
			++address;
			++octet_index;
			continue;
		}

		if (!isdigit(*address)) return 0;

		octets[octet_index] *= 10;
		octets[octet_index] += *address - ZERO_LITERAL;

		++address;
	}

	return 1;
}

static void ipv4_address_from_octets(uint32_t* to, u8 from[4]) {
	*to = 0;

	*to = from[0] << 24;
	*to |= from[1] << 16;
	*to |= from[2] << 8;
	*to |= from[3] & 0xFF;
}

void sockets_ipv4_init(struct sockets_address* address) {
	memset(address, 0, sizeof(struct sockets_address));
}

static void translate_address_to_sockaddr(struct sockets_address* address, struct sockaddr_in* out_address) {
#ifdef WIN32
	out_address->sin_addr.S_un.S_addr = htonl(address->ip);
	out_address->sin_port = htons(address->port);
	out_address->sin_family = AF_INET;
#else
#error "your platform is not supported"
#endif
}

i8 sockets_ipv4_from_string(struct sockets_address* address, const char* ipv4_text) {
	u8 octets[4];

	if (!parse_ipv4_address(ipv4_text, octets)) return 0;

	ipv4_address_from_octets(&address->ip, octets);

	return 1;
}

i8 sockets_bind(fd_t fd, struct sockets_address* address) {
	struct sockaddr_in addr = { 0 };
	translate_address_to_sockaddr(address, &addr);

	return bind((SOCKET)fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0 ? 0 : 1;
}

i8 sockets_listen(fd_t fd) {
	return listen((SOCKET)fd, SOMAXCONN) != 0 ? 0 : 1;
}

fd_t sockets_accept(fd_t server_fd) {
	fd_t accepted = (fd_t)accept((SOCKET)server_fd, NULL, NULL);

	return accepted == (fd_t)INVALID_SOCKET ? 0 : accepted;
}

i32 sockets_send(fd_t fd, const u8* buffer, usize size) {
	return send((SOCKET)fd, (const char*)buffer, (int)size, 0);
}

fd_t sockets_make_server(struct sockets_address* address) {
	fd_t new_server = sockets_new(SOCKET_TCP);

	if (!new_server) return 0;

	if (!sockets_bind(new_server, address)) return 0;

	if (!sockets_listen(new_server)) return 0;

	return new_server;
}