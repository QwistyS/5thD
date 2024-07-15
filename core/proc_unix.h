#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>

#include "5thdlogger.h"
#include "iproc_manager.h"

class UnixProcessManager : public IProcessManager {
public:
    UnixProcessManager() : _error(_drp) { _init(); }

    VoidResult start_proc(const std::string& name, const std::string& command) override {
        if (pid_t pid = fork(); pid == 0) {
            // Child process
            execl("/bin/sh", "sh", "-c", command.c_str(), (char*) nullptr);
            exit(1);
        } else if (pid > 0) {
            // Parent process
            _processes[name] = pid;
            DEBUG("Proccess {} started", name);
        } else {
            return Err(ErrorCode::FAIL_START_PROC, "Fail to start proccess ");
        }
        return Ok();
    }

    void stop_proc(const std::string& name) override {
        auto it = _processes.find(name);
        if (it != _processes.end()) {
            kill(it->second, SIGTERM);
            DEBUG("Stopped proc pid {}", it->second);
            _processes.erase(it);
        } else {
            WARN("Proccess {} not found", name);
        }
    }

    void stop_all() override {
        for (auto it = _processes.begin(); it != _processes.end();) {
            stop_proc(it->first);
            it = _processes.begin();
        }
    }

    long long get_pid(const std::string& name) override {
        if (auto it = _processes.find(name); it != _processes.end()) {
            return static_cast<long long>(it->second);
        }
        return -1;
    }

private:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;
    std::map<std::string, pid_t, std::less<>> _processes;
    void _init() {};
};

inline std::unique_ptr<IProcessManager> create_proc_manager() {
    return std::make_unique<UnixProcessManager>();
}