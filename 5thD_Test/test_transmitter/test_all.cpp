#include <cwchar>
#include <memory>

#include <zmq.h>
#include "izmq.h"
#include "receiver.h"
#include "transmitter.h"
#include "unity.h"

std::unique_ptr<ZMQWContext> context;

std::unique_ptr<ZMQWSocket<ZMQWContext>> trans_sock;
std::unique_ptr<ZMQWSocket<ZMQWContext>> recv_sock;

std::unique_ptr<ZMQWTransmitter<ZMQWSocket<ZMQWContext>>> trans;
std::unique_ptr<ZMQWReceiver<ZMQWSocket<ZMQWContext>>> recv;

int port;
std::string endpoint;

void setUp(void) {
    port = 3434;
    endpoint = "127.0.0.1";

    context = std::make_unique<ZMQWContext>();

    recv_sock = std::make_unique<ZMQWSocket<ZMQWContext>>(*context, ZMQ_REP);
    trans_sock = std::make_unique<ZMQWSocket<ZMQWContext>>(*context, ZMQ_REQ);

    trans = std::make_unique<ZMQWTransmitter<ZMQWSocket<ZMQWContext>>>(*trans_sock, "test_transmitter");
    recv = std::make_unique<ZMQWReceiver<ZMQWSocket<ZMQWContext>>>(endpoint, port, *recv_sock);

    recv->listen();
}

void tearDown(void) {
    // clang-format off
    trans->disconnect(endpoint, port);   // Disconnect
    trans_sock.reset();     // Reset socket
    trans.reset();      // Ensure transmitter close first

    recv->close();       // Unbind
    recv_sock.reset();   // Reset the socket before closing the context
    recv.reset();        // Ensure receiver is properly closed first

    context->close();   // Close the context
    context.reset();    // Reset the context
    // clang-format on
}

void test_ZMQWTransmitter_connect(void) {
    TEST_ASSERT(trans->connect(endpoint, port));
    TEST_ASSERT(trans->disconnect(endpoint, port));
}

void test_ZMQWTransmitter_send(void) {
    int data = 0xFFFFFFFF;
    trans->connect(endpoint, port);
    std::atomic<bool> until = true;

    // Create and start the worker thread
    auto t = std::thread(&ZMQWReceiver<ZMQWSocket<ZMQWContext>>::worker, recv.get(), &until, nullptr);

    // Send the message
    auto ret = trans->send(reinterpret_cast<const std::byte*>(&data), sizeof(data));
    TEST_ASSERT(ret);

    // Allow some time for the message to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Signal the worker thread to stop and join the thread
    until = false;
    if (t.joinable()) {
        t.join();
    }
}

int main(void) {
    Log::init();

    UNITY_BEGIN();
    RUN_TEST(test_ZMQWTransmitter_connect);
    RUN_TEST(test_ZMQWTransmitter_send);
    return UNITY_END();
}
