#include "module.h"
#include "5thdallocator.h"
#include "5thderror_handler.h"
#include "5thdsql.h"
#include "izmq.h"

bool KeysInfo::init() {
    bool ret = false;

    switch (KeysInfo::key_type) {
        case KeyType::CURVE25519:
            curve_pub = (char*) allocate_smem(41);
            curve_prv = (char*) allocate_smem(41);
            if (!curve_pub || !curve_prv) {
                ERROR("Mem alloc fail");
            } else {
                if ((sodium_mlock(curve_pub, 41) != (int) ErrorCode::OK)
                    || (sodium_mlock(curve_prv, 41) != (int) ErrorCode::OK)) {
                    ERROR("Fail to lock mem");
                    free_smem(curve_pub, 41);
                    free_smem(curve_prv, 41);
                }
                ret = true;
            }
            break;
        default:
            break;
    }
    return ret;
}

void KeysInfo::deinit() {
    switch (KeysInfo::key_type) {
        case KeyType::CURVE25519:
            free_smem(curve_pub, 41);
            free_smem(curve_prv, 41);
            break;

        default:
            break;
    }
}

inline bool _get_zmq_curve_keys(DatabaseAccess& db, char* pub, char* prv, Clients id) {
    DEBUG("Attempting to retrieve existing keys");
    auto public_key_result = get_key(db, CLIENTS_IDS[static_cast<int>(id)], KeyType::CURVE25519, "public_key");
    auto private_key_result = get_key(db, CLIENTS_IDS[static_cast<int>(id)], KeyType::CURVE25519, "private_key");

    if (public_key_result.is_err() || private_key_result.is_err()) {
        DEBUG("Keys not found, generating new ones");

        return false;
    }

    std::memcpy(pub, public_key_result.value().data(), public_key_result.value().size());
    std::memcpy(prv, private_key_result.value().data(), private_key_result.value().size());

    return true;
}

inline bool _handle_keys(void* conf) {
    module_init_t* config = (module_init_t*) conf;
    bool ret = false;
    DatabaseAccess db;

    switch (config->keys_info.key_type) {
        case KeyType::CURVE25519:
            config->keys_info.init();

            // New keys needed
            if (!_get_zmq_curve_keys(db, config->keys_info.curve_pub, config->keys_info.curve_prv, config->client_id)) {
                if (generate_keys(config->keys_info.curve_pub, config->keys_info.curve_prv) != (int) ErrorCode::OK) {
                    ERROR("Fail o curve keys");
                    config->keys_info.deinit();
                    return ret;
                } else {
                    std::vector<unsigned char> pub(config->keys_info.curve_pub,
                                                   config->keys_info.curve_pub
                                                       + std::strlen(config->keys_info.curve_pub));
                    std::vector<unsigned char> prv(config->keys_info.curve_prv,
                                                   config->keys_info.curve_prv
                                                       + std::strlen(config->keys_info.curve_prv));
                    auto begin_result = db.begin_transaction();
                    if (begin_result.is_err()) {
                        ERROR("Failed to begin transaction: {}", begin_result.error().message());
                        break;
                    }
                    auto write_pub = store_key(db, CLIENTS_IDS[static_cast<int>(config->client_id)],
                                               config->keys_info.key_type, "public_key", pub);
                    auto write_prv = store_key(db, CLIENTS_IDS[static_cast<int>(config->client_id)],
                                               config->keys_info.key_type, "private_key", prv);
                    if (write_prv.is_err() || write_prv.is_err()) {
                        db.exec("ROLLBACK");
                        ERROR("Error to write keys to db");
                        break;
                    }
                    auto commit_ret = db.end_transaction();
                    if (commit_ret.is_err()) {
                        ERROR("Failed to commit transaction: {}", commit_ret.error().message());
                        break;
                    }

                    DEBUG("Keys generated");
                }
                ret = true;
                break;

                default:
                    break;
            }
    }
    config->keys_info.is_ready = ret;
    return ret;
}

VoidResult module_init(module_init_t* config) {
    Log::init();
    init_sodium();
    atomic_cb wrapped_callback = std::bind(_handle_keys, std::placeholders::_1);

    handle_atomic_opt(wrapped_callback, (void*) config);
    return Ok();
}

VoidResult module_deinit(module_init_t* config) {
    for (auto it = config->unique_ptrs.rbegin(); it != config->unique_ptrs.rend(); ++it) {
        it->reset();
    }
    return Ok();
}
