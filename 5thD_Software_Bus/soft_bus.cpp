#include <zmq.h>
#include <memory>

#include "keys_db.h"
#include "module.h"
#include "software_bus.h"

int main() {
    module_init_t config;
    memset(&config, 0, sizeof(module_init_t));
    config.keys_info.is_ready = false;
    config.keys_info.key_type = KeyType::CURVE25519;
    config.client_id = Clients::ROUTER;

    module_init(&config);

    std::unique_ptr<ZMQWContext> ctx = std::make_unique<ZMQWContext>();
    std::unique_ptr<ZMQWSocket> socket = std::make_unique<ZMQWSocket>(ctx.get(), ZMQ_ROUTER);
    std::unique_ptr<ZMQWReceiver> recv =
        std::make_unique<ZMQWReceiver>(CLIENTS_IDS[static_cast<int>(config.client_id)], 0, ctx.get(), socket.get());
    recv->set_endpoint(IPC_ENDPOINT);
    std::unique_ptr<ZMQBus> bus = std::make_unique<ZMQBus>(recv.get());

    config.unique_ptrs.emplace_back(ctx.get(), Deleter());
    config.unique_ptrs.emplace_back(socket.get(), Deleter());
    config.unique_ptrs.emplace_back(recv.get(), Deleter());
    config.unique_ptrs.emplace_back(bus.get(), Deleter());

    signal(SIGINT, bus->signal_handler);
    signal(SIGTERM, bus->signal_handler);

    bus->set_security(config.keys_info.curve_pub, config.keys_info.curve_prv);
    config.keys_info.deinit();

    bus->run();

    module_deinit(&config);
    DEBUG("Clearing router...");
    return 0;
}
