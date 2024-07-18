#ifndef TRACY_WRAPPER_H
#define TRACY_WRAPPER_H

#ifdef TRACY_ENABLE
#    include <atomic>
#    include <tracy/Tracy.hpp>

class TracyWrapper {
public:
    static TracyWrapper& getInstance() {
        static TracyWrapper instance;
        return instance;
    }

    void setProfilingEnabled(bool enabled) { profilingEnabled.store(enabled); }

    bool isProfilingEnabled() const { return profilingEnabled.load(); }

private:
    TracyWrapper() : profilingEnabled(false) {}
    std::atomic<bool> profilingEnabled;
};

#    define TELEMETRY(msg)                                      \
        if (TracyWrapper::getInstance().isProfilingEnabled()) { \
            ZoneScoped;                                         \
        }

#else

class TracyWrapper {
public:
    static TracyWrapper& getInstance() {
        static TracyWrapper instance;
        return instance;
    }

    void setProfilingEnabled(bool enabled) {}
    bool isProfilingEnabled() const { return false; }
};

#    define TELEMETRY(msg)

#endif  // TRACY_ENABLE

#endif  // TRACY_WRAPPER_H
