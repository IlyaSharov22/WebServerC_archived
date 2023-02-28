#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "scalar.h"
#include "buffer.h"

typedef HANDLE fd_t;

#pragma comment(lib, "ws2_32.lib")