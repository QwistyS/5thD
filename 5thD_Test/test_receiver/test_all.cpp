#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "receiver.h"
#include "unity.h"


std::unique_ptr<ZMQWContext> context;
std::unique_ptr<ZMQWSocket> socket;
std::unique_ptr<ZMQWReceiver> recv;
int port;
std::string endpoint;

void setUp(void) {
    port = 3434;
    endpoint = "127.0.0.1";
    context = std::make_unique<ZMQWContext>();
    socket = std::make_unique<ZMQWSocket>(context.get(), ZMQ_REP);
    recv = std::make_unique<ZMQWReceiver>(endpoint, port, context.get(), socket.get());
}

void tearDown(void) {
    recv.release();
    socket.release();
    context.release();
}

void test_ZMQWRecv_get_port(void) {
    TEST_ASSERT_EQUAL_INT32(port, recv->get_port());
}

void test_ZMQWRecv_listen(void) {
    TEST_ASSERT(recv->listen());
}


int main(void) {
    Log::init();
    
    UNITY_BEGIN();
    RUN_TEST(test_ZMQWRecv_get_port);
    RUN_TEST(test_ZMQWRecv_listen);
    return UNITY_END();
}