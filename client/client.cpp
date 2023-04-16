#include "client.h"

int main(int argc, char* argv[]) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;

    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    int break_flag = 0;
    do {
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

            // Create a SOCKET for connecting to server
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                printf("socket failed with error: %ld\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }

            // Connect to server.
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            } else {
                break_flag = 1;
            }
            break; //got a connection, or none worked
        }
    } while(!break_flag);

    if(!break_flag) return 1;
    printf("Connection established!");
    do {
        char recvbuf[DEFAULT_BUFLEN];
        char response[DEFAULT_BUFLEN];

        memset(recvbuf, 0, DEFAULT_BUFLEN);
        iResult = recvMsg(ConnectSocket, recvbuf);

        if (iResult > 0) {
            printf("Received: %s", recvbuf);
            iResult = parseCommand(ConnectSocket, recvbuf);
            if(iResult==0) {
                strcpy(response, "OK");
                sendMsg(ConnectSocket, response);
            } else if (iResult==1) {
                strcpy(response, "BAD COMMAND");
                sendMsg(ConnectSocket, response);
            } else if (iResult==2) {
                strcpy(response, "GOODBYE");
                sendMsg(ConnectSocket, response);
                iResult = shutdown(ConnectSocket, SD_SEND);
                if (iResult == SOCKET_ERROR) {
                    printf("shutdown failed: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                closesocket(ConnectSocket);
                WSACleanup();
                break;
            } else {
                return 1;
            }
        }
    } while(1);
}

int parseCommand(SOCKET sock, char* input) {
    char* strippedInput = strtok(input, "\n");

    char* token = strtok(strippedInput, CMD_DELIMITER);
    char* command = token;
    char* args[MAX_ARGS];

    int argCount = 0;
    while (token != NULL) {
        args[argCount++] = token;
        token = strtok(NULL, CMD_DELIMITER);
    }

    if (strcmp(command, "shutdown") == 0) {
        return cmdShutdown();
    } else if (strcmp(command, "inform") == 0) {
        return cmdInform(sock);
    } else if (strcmp(command, "proc") == 0) {
        return cmdProc(sock);
    } else if (strcmp(command, "upload") == 0) {
        if (argCount == 2) {
            return cmdUpload(sock, args[1]);
        } else {
            printf("Invalid arguments for upload command.\n");
            return 1;
        }
    } else if (strcmp(command, "download") == 0) {
        if (argCount == 3) {
            return cmdDownload(args[1], args[2]);
        } else {
            printf("Invalid arguments for download command.\n");
            return 1;
        }
    } else {
        printf("Invalid command.\n");
        return 1;
    }
}

int cmdShutdown() {
    printf("Executing shutdown\n");
    return 2;
}

int cmdInform(SOCKET sock) {
    printf("Executing inform\n");
}

int cmdProc(SOCKET sock) {
    printf("Executing proc\n");
}

int cmdUpload(SOCKET sock, char* filepath) {
    printf("Executing cmdUpload() with arg: %s\n", filepath);
}

int cmdDownload(char* filename, char* url) {
    printf("Executing cmdDownload() with args: %s, %s\n", filename, url);
}