#include <endian.h>
#include <unity.h>
#include <memory>
#include <string>

#include "iproc_manager.h"

#ifdef _WIN32
#    include "proc_win.h"
#    include "win_implementation.h"
#elif defined(__linux__)
#    include "proc_unix.h"
#elif defined(__APPLE__)
#else
#    error "Unknown operating system"
#endif

std::unique_ptr<IProcessManager> proc;

std::string rabbit;

void setUp(void) {
    rabbit.assign(BIN_PATH "test_izmq");
    proc = create_proc_manager();
}

void tearDown(void) {
    proc.reset();
}

void test_proc_start(void) {
    auto ret = proc->start_proc(rabbit, "");
    TEST_ASSERT(ret.is_ok());
}

int main(void) {
    Log::init();
    UNITY_BEGIN();
    RUN_TEST(test_proc_start);
    return UNITY_END();
}
