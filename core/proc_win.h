#include "iproc_manager.h"
#include <iostream>
#include <map>
#include <stdexcept>
#include <windows.h>

class WindowsProcessManager : public IProcessManager {
private:
    struct ProcessInfo {
        PROCESS_INFORMATION processInfo;
    };
    
    std::map<std::string, ProcessInfo> processes;

public:
    void start_proc(const std::string& name, const std::string& command) override {
        STARTUPINFO startupInfo = {sizeof(startupInfo)};
        PROCESS_INFORMATION processInfo;
        
        if (!CreateProcess(NULL, const_cast<LPSTR>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
            throw std::runtime_error("CreateProcess failed");
        }
        
        processes[name].processInfo = processInfo;
        std::cout << "Started " << name << " with PID " << processInfo.dwProcessId << std::endl;
    }

    void stop_proc(const std::string& name) override {
        auto it = processes.find(name);
        if (it != processes.end()) {
            TerminateProcess(it->second.processInfo.hProcess, 0);
            CloseHandle(it->second.processInfo.hProcess);
            CloseHandle(it->second.processInfo.hThread);
            std::cout << "Stopped " << name << " with PID " << it->second.processInfo.dwProcessId << std::endl;
            processes.erase(it);
        } else {
            std::cout << "Process " << name << " not found" << std::endl;
        }
    }

    void stop_all() override {
        for (auto it = processes.begin(); it != processes.end();) {
            stopProcess(it->first);
            it = processes.begin();
        }
    }

    long long get_pid(const std::string& name) override {
        auto it = processes.find(name);
        if (it != processes.end()) {
            return static_cast<long long>(it->second.processInfo.dwProcessId);
        }
        return 0;
    }
};

IProcessManager* create_proc_manager() {
    return new WindowsProcessManager();
}