#include <stdio.h>
#include <zmq.h>
#include "5thdipc_client.h"
#include "5thdlogger.h"

int main(int argc, char* argv[]) {
    Log::init();
    char command[10];
    ipc_msg_t data;

    ZMQWContext ctx;
    ZMQWSocket socket(&ctx, ZMQ_DEALER); 
    ZMQTransmitter trans(&ctx, &socket, "manager");
    IpcClient ipc_client(&trans);

    data.dist_id = 1;
    data.src_id = 1;



    while (1) {
        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("Error reading input");
            return 1;
        }
        if (command[0] == 'q') {
            break;
        }
        ipc_client.send(&data);
    }

    return 0;
}
