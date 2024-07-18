#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "unity.h"

std::unique_ptr<ZMQWContext> context;
std::unique_ptr<ZMQWSocket<ZMQWContext>> trans_sock;
int port = 0;
std::string endpoint;

void setUp(void) {
    port = 3434;
    endpoint = "120.0.0.1";
    context = std::make_unique<ZMQWContext>();
    trans_sock = std::make_unique<ZMQWSocket<ZMQWContext>>(*context, ZMQ_REP);
}

void tearDown(void) {
    trans_sock.reset();
    context->close();
    context.reset();
}

void test_ZMQWContext(void) {
    TEST_ASSERT_NOT_NULL(context->get_context());
}

void test_ZMQWSocket(void) {
    TEST_ASSERT_NOT_NULL(trans_sock->get_socket());
}

int main(void) {
    Log::init();
    UNITY_BEGIN();
    RUN_TEST(test_ZMQWContext);
    RUN_TEST(test_ZMQWSocket);
    return UNITY_END();
}
