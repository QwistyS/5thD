cmake_minimum_required(VERSION 3.20)
project(5thDTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
)

file(GLOB TESTS 
    "core/src/*.cpp"
    "../core/receiver.cpp"
    "../core/izmq.cpp"
    "../core/5thdlogger.cpp"
    "../core/5thdipcmsg.c"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    libzmq
    spdlog::spdlog 
    ${LIBSODIUM_LIBRARIES}
    unity
)
