#ifndef IPROC_MANAGER_H
#define IPROC_MANAGER_H

#include <string>

class IProcessManager {
public:
    virtual ~IProcessManager() = default;
    virtual void start_proc(const std::string& name, const std::string& command) = 0;
    virtual void stop_proc(const std::string& name) = 0;
    virtual void stop_all() = 0;
    virtual long long get_pid(const std::string& name) = 0;
};

IProcessManager* create_proc_manager();

#endif // IPROC_MANAGER_H