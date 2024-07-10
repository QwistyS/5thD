#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "unity.h"


std::unique_ptr<ZMQWContext> context;
std::unique_ptr<ZMQWSocket> socket;
int port;
std::string endpoint;

void setUp(void) {
    port = 3434;
    endpoint = "120.0.0.1";
    context = std::make_unique<ZMQWContext>();
    socket = std::make_unique<ZMQWSocket>(context.get(), ZMQ_REP);
}

void tearDown(void) {
    socket.release();
    context.release();
}

void test_ZMQWContext(void) {
    TEST_ASSERT_NOT_NULL(context->get_context());
}

void test_ZMQWSocket(void) {
    TEST_ASSERT_NOT_NULL(socket->get_socket());
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ZMQWContext);
    RUN_TEST(test_ZMQWContext);
    return UNITY_END();
}