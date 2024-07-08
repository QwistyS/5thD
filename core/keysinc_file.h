#ifndef KEYS_INC_FILE_H
#define KEYS_INC_FILE_H

#include <string>
#include <vector>

class KeysIncFile {
public:
    enum class KeyType : uint16_t {
        ZMQ_CURVE = 1,
        API_KEY = 2,
        USER_PASSWORD_HASH = 3,
        ENCRYPTION_KEY = 4,
        SIGNING_KEY = 5,
    };

    struct KeyEntry {
        uint16_t key_type;
        uint8_t key_id[256];
        uint64_t timestamp;
        uint16_t version;
        uint8_t metadata[16];
        uint16_t key_length;
        uint8_t key_data[256];
    };

    KeysIncFile(const std::string& fname);
    ~KeysIncFile() = default;

    void add_key(const std::string& domain, const KeyEntry& key);
    std::vector<KeyEntry> get_keys_by_domain(const std::string& domain);
    bool update_key(const std::string& domain, const KeyEntry& key);
    bool remove_key(const std::string& domain, const uint8_t* key_id);

    void validate();

private:
    struct FileHeader {
        uint32_t magic_number;
        uint16_t version;
        uint32_t index_table_offset;
        uint32_t index_table_length;
        uint64_t file_length;
    };

    struct IndexEntry {
        char domain[64];
        uint64_t offset;
        uint32_t count;
    };

    std::string _filename;
    FileHeader _header;
    std::vector<IndexEntry> _index_table;

    void _read_header();
    void _write_header();
    void _read_index_table();
    void _write_index_table();
    uint32_t _calculate_crc();
    void _update_crc();
};

#endif  // KEYS_INC_FILE_H