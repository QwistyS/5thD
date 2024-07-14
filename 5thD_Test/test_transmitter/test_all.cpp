#include <cwchar>
#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "receiver.h"
#include "transmitter.h"
#include "unity.h"


std::unique_ptr<ZMQWContext> context;
std::unique_ptr<ZMQWSocket> socket;
std::unique_ptr<ZMQWSocket> srv_sock;
std::unique_ptr<ZMQWTransmitter> trans;
std::unique_ptr<ZMQWReceiver> srv;
int port;
std::string endpoint;

void setUp(void) {
    port = 3434;
    endpoint = "127.0.0.1";
    context = std::make_unique<ZMQWContext>();
    srv_sock = std::make_unique<ZMQWSocket>(context.get(), ZMQ_REP);
    socket = std::make_unique<ZMQWSocket>(context.get(), ZMQ_REQ);
    trans = std::make_unique<ZMQWTransmitter>(context.get(), socket.get(), "test_transmitter");
    srv = std::make_unique<ZMQWReceiver>(endpoint, port, context.get(), srv_sock.get());
}

void tearDown(void) {
    srv.reset();
    trans.reset();
    socket.reset();
    srv_sock.reset();
    context.reset();
}

void test_ZMQWTrans_connect(void) {
    srv->listen();
    TEST_ASSERT(trans->connect(endpoint, port));
}

void test_ZMQWTrans_send(void) {
    int data = 0xFFFFFFFF;
    srv->listen();
    trans->connect(endpoint, port);

    auto ret =trans->send(&data, sizeof(data));
    TEST_ASSERT(ret);
}

int main(void) {
    Log::init();
    
    UNITY_BEGIN();
    RUN_TEST(test_ZMQWTrans_connect);
    RUN_TEST(test_ZMQWTrans_send);
    return UNITY_END();
}
