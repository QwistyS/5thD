# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(core/inc)

# Define options for selecting the target platform
option(PLATFORM_UNIX "Build for Unix platforms (Linux, macOS)" OFF)
option(PLATFORM_MAC "Build for macOS" OFF)
option(PLATFORM_IOS "Build for iOS" OFF)
option(PLATFORM_ANDROID "Build for Android" OFF)

# Set default target platform if none selected
if (NOT PLATFORM_UNIX AND NOT PLATFORM_MAC AND NOT PLATFORM_IOS AND NOT PLATFORM_ANDROID)
    set(PLATFORM_UNIX ON)
endif()

# Set compiler flags and target-specific settings based on the selected platform
if (PLATFORM_UNIX)
    # Unix platform settings
    message(STATUS "Building for Unix platforms (Linux, macOS)")
elseif (PLATFORM_MAC)
    # macOS platform settings
    message(STATUS "Building for macOS")
    # Add macOS-specific configuration here
elseif (PLATFORM_IOS)
    # iOS platform settings
    message(STATUS "Building for iOS")
    # Add iOS-specific configuration here
elseif (PLATFORM_ANDROID)
    # Android platform settings
    message(STATUS "Building for Android")
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a" CACHE STRING "")
    set(CMAKE_ANDROID_NDK /path/to/your/android/ndk CACHE STRING "")
    set(ANDROID_PLATFORM android-30 CACHE STRING "")
    set(ANDROID_STL c++_shared CACHE STRING "")
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "")
    set(CMAKE_ANDROID_API_MIN 23 CACHE STRING "")
    set(CMAKE_ANDROID_API 30 CACHE STRING "")
    set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang CACHE STRING "")
    set(CMAKE_ANDROID_STL_TYPE c++_shared CACHE STRING "")
    set(CMAKE_ANDROID_NDK_PLATFORM android-${CMAKE_ANDROID_API} CACHE STRING "")
endif()

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add all .cpp files from core/src directory
file(GLOB CORE_SRC_FILES "core/src/*.cpp")

# Add Unity library
add_subdirectory(path/to/Unity)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${CORE_SRC_FILES})

# Link Unity library to the main executable
target_link_libraries(${PROJECT_NAME} Unity)

# Find the Qt6 package
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Link Qt6 libraries to the main executable
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Gui Qt6::Widgets)

# Add an executable target for the tests
add_executable(5thDTest 5thDTest/test_network.c)

# Include directories for the test executable
target_include_directories(5thDTest PRIVATE core/inc)

# Link Unity library to the test executable
target_link_libraries(5thDTest Unity)

# Link the core module to the test executable
target_link_libraries(5thDTest thread_pool)
