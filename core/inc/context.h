#ifndef CONTEXT_H
#define CONTEXT_H

class IContext {
public:
    virtual ~IContext() = default;
    virtual void* get_context() = 0;
};

/**
 * @brief Wrapper for zmq context
 */
class Context : public IContext {
public:
    virtual ~Context();
    Context() : _context(nullptr) { set_context(); };
    void* get_context();

private:
    void close();
    void set_context();
    void* _context;
};

#endif  // CONTEXT_H