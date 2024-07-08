#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <stdexcept>

#include "5thderror_handler.h"
#include "iproc_manager.h"

class UnixProcessManager : public IProcessManager {
public:
    UnixProcessManager() : _error(_drp) { _init(); }

    void start_proc(const std::string& name, const std::string& command) override {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execl("/bin/sh", "sh", "-c", command.c_str(), (char*) nullptr);
            exit(1);
        } else if (pid > 0) {
            // Parent process
            _processes[name] = pid;
            std::cout << "Started " << name << " with PID " << pid << std::endl;
        } else {
            throw std::runtime_error("Fork failed");
        }
    }

    void stop_proc(const std::string& name) override {
        auto it = _processes.find(name);
        if (it != _processes.end()) {
            kill(it->second, SIGTERM);
            std::cout << "Stopped " << name << " with PID " << it->second << std::endl;
            _processes.erase(it);
        } else {
            std::cout << "Process " << name << " not found" << std::endl;
        }
    }

    void stop_all() override {
        for (auto it = _processes.begin(); it != _processes.end();) {
            stop_proc(it->first);
            it = _processes.begin();
        }
    }

    long long get_pid(const std::string& name) override {
        auto it = _processes.find(name);
        if (it != _processes.end()) {
            return static_cast<long long>(it->second);
        }
        return -1;
    }

protected:
    ErrorHandler _error;
    DisasterRecoveryPlan _drp;

private:
    std::map<std::string, pid_t> _processes;
    void _init() {};
};

IProcessManager* create_proc_manager() {
    return new UnixProcessManager();
}