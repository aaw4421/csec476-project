#include "server.h"

DWORD WINAPI handleClient(LPVOID lpParam) {
    SOCKET ClientSocket = *(SOCKET*)lpParam;

    int result; 

    do {
        char response[DEFAULT_BUFLEN];
        char message[DEFAULT_BUFLEN];

        memset(response, 0, DEFAULT_BUFLEN);

        printf("\ncommand: ");
        fgets(message, DEFAULT_BUFLEN, stdin);

        result = sendMsg(ClientSocket, message);
        printf("sent: %s", message);
        result = recvMsg(ClientSocket, response);
        printf("received: %s", response);
    } while(1);

    printf("Disconnecting client\n");

    closesocket(ClientSocket);
}

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo *result = NULL, 
                    *ptr = NULL, 
                    hints;

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    // Start listening
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while(1) {
        ClientSocket = accept(ListenSocket, NULL, NULL);

        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        printf("Connection opened with client...");
        CreateThread(NULL, 0, handleClient, &ClientSocket, 0, NULL);
    }

    closesocket(ListenSocket);

    WSACleanup();

    return 0;
}