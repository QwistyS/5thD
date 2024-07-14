#ifndef MANAGED_BUFFER_H
#define MANAGED_BUFFER_H

#include <zmq.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include "5thderror_handler.h"

template <typename T, size_t Size>
class ManagedBuffer {
public:
    explicit ManagedBuffer(std::function<void(T&)> init_func, std::function<void(T&)> deinit_func);
    ManagedBuffer(const ManagedBuffer&) = delete;
    ManagedBuffer& operator=(const ManagedBuffer&) = delete;
    ~ManagedBuffer();

    /**
     * @brief
     */
    T* get_slot();
    
    /**
     * @brief
     */
    Result<bool> release_slot(T** slot_ptr);
    
    /**
     * @brief
     */
    Result<bool> is_empty() const;
    
    /**
     * @brief
     */
    Result<bool> is_full() const;
    
    /**
     * @brief
     */
    Result<size_t> size() const;
    
    /**
     * @brief
     */
    constexpr size_t capacity() const;
    
    /**
     * @brief
     */
    VoidResult check_integrity() const;

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    static constexpr uint64_t CANARY_VALUE = 0xDEADBEEFCAFEBABE;

    struct Metadata {
        bool in_use = false;
        size_t id = 0;
        size_t next = 0;
    };

    struct Slot {
        Metadata meta;
        T data;
        uint64_t canary = CANARY_VALUE;
    };

    std::array<Slot, Size> _buffer;
    mutable std::mutex _mutex;
    std::function<void(T&)> _user_data_init_callback;
    std::function<void(T&)> _user_data_deinit_callback;
    size_t current_free = 0;
    size_t used_count = 0;

    VoidResult check_canary(const Slot* slot) const;
    void _init();
    void _setup_drp();
    bool _handle_canary();
};

template <typename T, size_t Size>
ManagedBuffer<T, Size>::ManagedBuffer(std::function<void(T&)> init_func, std::function<void(T&)> deinit_func)
    : _user_data_init_callback(std::move(init_func)), _user_data_deinit_callback(std::move(deinit_func)), _error(_drp) {
    _init();
}

template <typename T, size_t Size>
inline ManagedBuffer<T, Size>::~ManagedBuffer() {
    for (size_t i = 0; i < _buffer.size(); ++i) {
        _user_data_deinit_callback(_buffer[i].data);
    }
}

template <typename T, size_t Size>
T* ManagedBuffer<T, Size>::get_slot() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (used_count == Size) {
        return nullptr;
    }

    size_t slot_index = current_free;
    current_free = _buffer[slot_index].meta.next;
    _buffer[slot_index].meta.in_use = true;

    ++used_count;
    return &_buffer[slot_index].data;
}

template <typename T, size_t Size>
Result<bool> ManagedBuffer<T, Size>::release_slot(T** slot_ptr) {
    if (slot_ptr == nullptr || *slot_ptr == nullptr) {
        return Err<bool>(ErrorCode::MANAGE_BUFF_NULL_ON_RELEASE, "Null was given to release");
    }

    std::lock_guard<std::mutex> lock(_mutex);

    Slot* slot = reinterpret_cast<Slot*>(reinterpret_cast<char*>(*slot_ptr) - offsetof(Slot, data));

    if (slot < &_buffer[0] || slot >= &_buffer[Size]) {
        return Err<bool>(ErrorCode::MANAGE_BUFF_SLOT_NOT_IN_RANGE, "Slot not in range of buffer memory");
    }

    if (!slot->meta.in_use) {
        return Err<bool>(ErrorCode::MANAGE_BUFF_MONKEY, "Slot is not in use");
    }

    auto canary_ret = check_canary(slot);
    if (canary_ret.is_err()) {
        if (!_error.handle_error(canary_ret.error())) {
            std::abort();
        }
    }

    size_t released_index = static_cast<size_t>(slot - &_buffer[0]);

    slot->meta.in_use = false;

    slot->meta.next = current_free;
    current_free = released_index;

    --used_count;
    *slot_ptr = nullptr;
    return Ok<bool>(true);
}

template <typename T, size_t Size>
Result<bool> ManagedBuffer<T, Size>::is_empty() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return Ok<bool>(used_count == 0);
}

template <typename T, size_t Size>
Result<bool> ManagedBuffer<T, Size>::is_full() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return Ok<bool>(used_count == Size);
}

template <typename T, size_t Size>
Result<size_t> ManagedBuffer<T, Size>::size() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return Ok<size_t>(used_count);
}

template <typename T, size_t Size>
constexpr size_t ManagedBuffer<T, Size>::capacity() const {
    return Size;
}

template <typename T, size_t Size>
VoidResult ManagedBuffer<T, Size>::check_integrity() const {
    std::lock_guard<std::mutex> lock(_mutex);
    for (const auto& slot : _buffer) {
        auto canary_ret = check_canary(&slot);
        if (canary_ret.is_err()) {
            if (!_error.handle_error(canary_ret.error())) {
                std::abort();
            }
        }
    }
    return Ok();
}

template <typename T, size_t Size>
VoidResult ManagedBuffer<T, Size>::check_canary(const Slot* slot) const {
    if (slot->canary != CANARY_VALUE) {
        return Err(ErrorCode::MANAGE_BUFF_OVERFLOW, "Buffer overflow detected", Severity::CRITICAL);
    }
    return Ok();
}

template <typename T, size_t Size>
void ManagedBuffer<T, Size>::_init() {
    _setup_drp();

    for (size_t i = 0; i < _buffer.size(); ++i) {
        _buffer[i].meta = {false, i, (i + 1) % _buffer.size()};
        _user_data_init_callback(_buffer[i].data);
        _buffer[i].canary = CANARY_VALUE;
    }
}

template <typename T, size_t Size>
void ManagedBuffer<T, Size>::_setup_drp() {
    _drp.register_recovery_action(ErrorCode::MANAGE_BUFF_OVERFLOW, [this]() {
        WARN("Buffer overflow detected");
        return _handle_canary();
    });
}

template <typename T, size_t Size>
bool ManagedBuffer<T, Size>::_handle_canary() {
    return false;
}

#endif  // MANAGED_BUFFER_H