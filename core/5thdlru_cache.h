#ifndef LRU_CHACHE_H
#define LRU_CHACHE_H

#include <openssl/sha.h>
#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

template <typename Tkey, typename Tval>
class LRU_Cache {
public:
    LRU_Cache(uint16_t max_size) : _max_size(max_size) {}

    void push(const Tkey& key, const Tval& value) {
        std::lock_guard<std::mutex> lock(_mutex);

        // case already exist
        auto it = _cache_items_map.find(key);
        if (it != _cache_items_map.end()) {
            // Remove the position at address in list
            _cache_items_list.erase(it->second);
            // push @ front
            _cache_items_list.push_front(std::make_pair(key, value));
            // Update the pointer
            _cache_items_map[key] = _cache_items_list.begin();
        } else {
            // New item
            if (_cache_items_map.size() >= _max_size) {
                auto end = _cache_items_list.end();
                end--;
                // remove last item from map
                _cache_items_map.erase(end->first);
                _cache_items_list.pop_back();
            }
            // Push @ front
            _cache_items_list.push_front(std::make_pair(key, value));
            _cache_items_map[key] = _cache_items_list.begin();
        }
    }

    bool get(const Tkey& key, Tval& value) {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _cache_items_map.find(key);
        if (it == _cache_items_map.end()) {
            return false;
        } else {
            // Move the accessed item to the front
            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            value = it->second->second;
            return true;
        }
    }

private:
    std::list<std::pair<Tkey, Tval>> _cache_items_list;
    std::unordered_map<Tkey, typename std::list<std::pair<Tkey, Tval>>::iterator> _cache_items_map;
    uint16_t _max_size;
    std::mutex _mutex;
};

#endif  // LRU_CHACHE_H