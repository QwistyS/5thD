
#include "izmq.h"
#include "receiver.h"
#include "software_bus.h"
#include "keys_db.h"
#include "module.h"

int main() {
    module_init_t config;
    memset(&config, 0, sizeof(module_init_t));
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;
    config.client_id = Clients::ROUTER;

    module_init(&config);

    auto ctx = std::make_unique<ZMQWContext>();
    auto socket = std::make_unique<ZMQWSocket<ZMQWContext>>(*ctx, ZMQ_ROUTER);

    auto recv = std::make_unique<ZMQWReceiver<ZMQWSocket<ZMQWContext>>>("", 0, *socket);
    recv->set_endpoint(IPC_ENDPOINT);

    ZMQBus<ZMQWReceiver<ZMQWSocket<ZMQWContext>>> bus(*recv);
    
    signal(SIGINT, bus.signal_handler);
    signal(SIGTERM, bus.signal_handler);

    bus.set_security(config.keys_info.curve_pub, config.keys_info.curve_prv);
    config.keys_info.deinit();

    bus.run();

    recv->close();

    DEBUG("Clearing router...");
    return 0;
}
