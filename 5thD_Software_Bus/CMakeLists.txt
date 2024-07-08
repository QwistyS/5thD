cmake_minimum_required(VERSION 3.20)
project(5thDSoftwareBus)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
)

file(GLOB SOFT_BUS_FILES 
    "core/src/*.cpp"
    "../core/receiver.cpp"
    "../core/izmq.cpp"
    "../core/5thdlogger.cpp"
    "../core/5thdipcmsg.c"
    "../core/5thdsql.cpp"
    "../core/keys_db.cpp"

)

set(SOURCES soft_bus.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${SOFT_BUS_FILES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    libzmq
    spdlog::spdlog
    OpenSSL::SSL
    ${LIBSODIUM_LIBRARIES}
    ${SQLCIPHER_LIBRARIES}
)
