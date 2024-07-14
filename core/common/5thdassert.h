#ifndef ASSERT_MACROS_H
#define ASSERT_MACROS_H

#include <cstdlib>  // For std::abort()

// Define the APP_ASSERT macro only if not in a release build
#ifndef NDEBUG
    #ifdef _MSC_VER
        // MSVC compiler
        #define APP_ASSERT(value) do { if (!(value)) __debugbreak(); } while (0)
    #elif defined(__GNUC__) || defined(__clang__)
        // GCC or Clang on Linux
        #define APP_ASSERT(value) do { if (!(value)) __builtin_trap(); } while (0)
    #else
        // Unknown compiler/platform
        #define APP_ASSERT(value) do { if (!(value)) std::abort(); } while (0)
    #endif
#else
    // In release builds, define APP_ASSERT as a no-op
    #define APP_ASSERT(value) ((void)0)
#endif

#endif // ASSERT_MACROS_H
