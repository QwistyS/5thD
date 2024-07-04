cmake_minimum_required(VERSION 3.20)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THIRDP "third_parties/")


# Include directories for core module headers
include_directories(core/
core/peer/inc
)

file(GLOB CORE_SRC_FILES 
"core/*.cpp"
"core/peer/src/*.cpp"
)


# Args parser
add_subdirectory(${THIRDP}/curl)
include_directories(${THIRDP}curl/include)

# Manually specify the include directories and library paths for libsodium
set(LIBSODIUM_INCLUDE_DIRS /usr/local/include)
set(LIBSODIUM_LIBRARIES /usr/local/lib/libsodium.dylib)

# Include the libsodium headers
include_directories(${LIBSODIUM_INCLUDE_DIRS})

# Set libzmq to build static libraries with CurveZMQ support
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ENABLE_DRAFTS OFF CACHE BOOL "Build and install draft classes and methods" FORCE)
set(ENABLE_CPACK OFF CACHE BOOL "Enable CPack packaging" FORCE)
set(WITH_PERF_TOOL OFF CACHE BOOL "Build performance tests" FORCE)
set(WITH_LIBSODIUM ON CACHE BOOL "Build with libsodium for encryption" FORCE)
set(ENABLE_CURVE ON CACHE BOOL "Build with libsodium for encryption" FORCE)
set(WITH_DOCS OFF CACHE BOOL "Build documentation" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "Build ZeroMQ tests" FORCE)
set(ENABLE_CZMQ_BUILD_TESTS OFF CACHE BOOL "Build CZMQ tests" FORCE)

# Add libzmq
add_subdirectory(${THIRDP}libzmq)
include_directories(${THIRDP}libzmq/include)

# Add miniupnpc
add_subdirectory(${THIRDP}miniupnpc)
include_directories(${THIRDP}miniupnpc/include)
# Add spdlog
add_subdirectory(${THIRDP}spdlog)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find Qt6 components
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${CORE_SRC_FILES})

# Link Qt6 libraries and other dependencies to the main executable
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    spdlog::spdlog
    libzmq
    ${LIBSODIUM_LIBRARIES}
)

# Ensure CMake uses the correct linker and include paths for cross-compiling
if(CMAKE_CROSSCOMPILING)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

# add_subdirectory(5thD_Peer)
add_subdirectory(5thD_Software_Bus)