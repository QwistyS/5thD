#ifndef KEYS_H
#define KEYS_H

#define KEY_LENGTH 41
#define PATH_TO_KEY "keys.inc"

#define PRIVATE_KEY_FLAG 0
#define PUBLIC_KEY_FLAG  1

typedef struct {
    char* public_key;
    char* private_key;
} keys_t;

class Keys {
public:
    Keys(const char* path) { _init(path); };
    ~Keys();
    static Keys* get_instance();
    const char* get_key(int flag);
private:
    keys_t _keys;
    void _init(const char* path);
};




#endif  // KEYS_H