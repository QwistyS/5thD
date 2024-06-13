#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    // Prepare our context and socket
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, "tcp://*:5555");

    while (1) {
        char buffer[10];
        zmq_recv(responder, buffer, 10, 0);
        printf("Received Hello\n");
        sleep(1); // Do some 'work'
        zmq_send(responder, "World", 5, 0);
    }
    // Clean up
    zmq_close(responder);
    zmq_ctx_destroy(context);
    return 0;
}