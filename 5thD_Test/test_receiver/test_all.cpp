#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "receiver.h"
#include "unity.h"
#include "unity_internals.h"

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
    recv = std::make_unique<ZMQWReceiver>(endpoint, port, socket.get());
}

void tearDown(void) {
    recv.reset();       // Ensure receiver is properly closed first
    socket.reset();     // Reset the socket before closing the context
    context->close();   // Close the context
    context.reset();    // Reset the context
}

void test_ZMQWRecv_set_endpoint(void) {
    TEST_ASSERT(recv->set_endpoint(endpoint.c_str()));
}

void test_ZMQWRecv_get_port(void) {
    TEST_ASSERT_EQUAL_INT(port, recv->get_port());
}

void test_ZMQWRecv_listen(void) {
    TEST_ASSERT(recv->listen());
    TEST_ASSERT(recv->close());
}

int main(void) {
    Log::init();

    UNITY_BEGIN();
    RUN_TEST(test_ZMQWRecv_set_endpoint);
    RUN_TEST(test_ZMQWRecv_get_port);
    RUN_TEST(test_ZMQWRecv_listen);
    return UNITY_END();
}
