#define WIN32_LEAN_AND_MEAN
#define SERVER_HOST "localhost"
#define SERVER_PORT "1337"
#define DEFAULT_BUFLEN 1024

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <WinSock2.h>

#pragma comment (lib, "Ws2_32.lib")

