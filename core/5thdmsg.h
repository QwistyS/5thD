#ifndef MSG_H
#define MSG_H

#include "5thdtask.h"
#include "connection.h"

typedef struct {
    conn_info_t conn_info;
    char msg[256];
    Task task;
} netmsg_t;


#endif // MSG_H