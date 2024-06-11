# CMakeLists.txt

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

# Set libzmq to build static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ENABLE_DRAFTS OFF CACHE BOOL "Build and install draft classes and methods" FORCE)
set(WITH_PERF_TOOL OFF CACHE BOOL "Build performance tests" FORCE)
set(WITH_LIBSODIUM ON CACHE BOOL "Build with libsodium for encryption" FORCE)
# Disable tests for libzmq
set(ENABLE_CPACK OFF CACHE BOOL "Enable CPack packaging" FORCE)
set(ENABLE_CZMQ_BUILD_TESTS OFF CACHE BOOL "Build CZMQ tests" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "Build ZeroMQ tests" FORCE)
set(WITH_DOCS OFF CACHE BOOL "Build documentation" FORCE)
# Add libzmq
add_subdirectory(${THIRDP}libzmq)
include_directories(${THIRDP}libzmq/include)

add_subdirectory(${THIRDP}miniupnpc)
include_directories(${THIRDP}miniupnpc/include)
# Find spdlog package
add_subdirectory(${THIRDP}spdlog)
# Find OpenSSl
find_package(OpenSSL REQUIRED)
# Find Qt stuff
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add all .cpp files from core/src directory

# Add an executable target for the main application
add_executable(${PROJECT_NAME} ${SOURCES} ${CORE_SRC_FILES})

# Find the Qt6 package

# Link Qt6 libraries to the main executable
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    spdlog::spdlog
    libzmq-static
    OpenSSL::SSL
    miniupnpc::miniupnpc
)

