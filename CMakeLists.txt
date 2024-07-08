cmake_minimum_required(VERSION 3.20)
project(5thD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THIRDP "third_parties/")

# Set the output directory for all targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)

# Include directories for core module headers
include_directories(
    core/common
    core
)

file(GLOB CORE_SRC_FILES 
    core/receiver.cpp
    core/transmitter.cpp
    core/izmq.cpp
    core/5thdlogger.cpp
    core/5thdipcmsg.c
    core/5thdipc_client.cpp
    core/5thdipcmsg.h
)


add_subdirectory(${THIRDP}/curl)
include_directories(${THIRDP}curl/include)

# Manually specify the include directories and library paths for libsodium
set(LIBSODIUM_INCLUDE_DIRS /usr/local/include)
set(LIBSODIUM_LIBRARIES /usr/local/lib/libsodium.dylib)
include_directories(${LIBSODIUM_INCLUDE_DIRS})

# Manually added sqlcipher
add_definitions(-DSQLITE_HAS_CODEC)
set(SQLCIPHER_LIBRARIES "/usr/local/lib/libsqlcipher.dylib")
set(SQLCIPHER_INCLUDE_DIRS "/usr/local/include/sqlcipher/sqlite3.h")

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

set(IMGUI_DIR ${THIRDP}imgui)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRDP}glfw)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Set the source files for your Qt application
set(SOURCES main.cpp)

# Add an executable target for the main application
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${CORE_SRC_FILES}
    ${IMGUI_SOURCES}
    )

# Link libraries and other dependencies to the main executable
if(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        spdlog::spdlog
        glfw
        OpenGL::GL
        libzmq
        ${LIBSODIUM_LIBRARIES}
        ${SQLCIPHER_LIBRARIES}
        "-framework Cocoa" 
        "-framework IOKit" 
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        spdlog::spdlog
        glfw
        OpenGL::GL
    )
endif()

# Ensure CMake uses the correct linker and include paths for cross-compiling
if(CMAKE_CROSSCOMPILING)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()


add_subdirectory(5thD_Software_Bus)
add_subdirectory(5thD_Peer)
add_subdirectory(5thDTest)