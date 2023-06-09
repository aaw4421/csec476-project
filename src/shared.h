#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WIN32_WINNT 0x501

#define DEFAULT_PORT "1337"
#define DEFAULT_BUFLEN 1024
#define SOMAXCONN 1
#define MAX_ARGS 2
#define CMD_DELIMITER "?"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

int sendMsg(SOCKET, char*);
int sendMsg(SOCKET, char*, int);
int recvMsg(SOCKET, char*);
int encryptBuffer(char*, char*, int);

int sendMsg(SOCKET sock, char* msg) {
    sendMsg(sock, msg, strlen(msg));
}

int sendMsg(SOCKET sock, char* msg, int len) {
    int result;

    char enc[DEFAULT_BUFLEN];
    memset(enc, 0, DEFAULT_BUFLEN);
    encryptBuffer(enc, msg, len);

    result = send(sock, enc, len, 0);
    if (result == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    return result;
}

int recvMsg(SOCKET sock, char* response) {
    memset(response, 0, DEFAULT_BUFLEN);

    char enc[DEFAULT_BUFLEN];
    memset(enc, 0, DEFAULT_BUFLEN);

    int result;
    result = recv(sock, enc, DEFAULT_BUFLEN, 0);
    if (result == SOCKET_ERROR) {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    encryptBuffer(response, enc, strlen(enc));
    return result;
}

int encryptBuffer(char* dest, char* src, int len) {
    const char secret[] = {0x01, 0x02, 0x03, 0x04};
    int encryptedBytes = 0;
    for(int i=0; i<len; i++) {
        dest[i] = src[i] ^ secret[i % strlen(secret)];
        encryptedBytes++;
    }
    return encryptedBytes;
}