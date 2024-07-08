#include "5thdipcmsg.h"

#ifdef _WIN32
const char* IPC_ENDPOINT = "ipc://secure_ipc";
#else
const char* IPC_ENDPOINT = "ipc:///tmp/secure_ipc";
#endif

const char* CLIENTS_IDS[] = {
    "manager",
    "peerxxx",
    "uixxxxx",
    "ipcrout",
};