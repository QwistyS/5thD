cmake_minimum_required(VERSION 3.20)
project(5thDPeer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Include directories for core module headers
include_directories(
    ../core/common
    core/inc
    /home/qwistys/src/5thD/third_parties/libzmq/include
    /home/qwistys/src/5thD/third_parties/sqlcipher/include
)


# Set the path for the library files
set(ZMQ_LIB ${5thDlibs}/libzmq.so)
set(SQLCIPHER_LIB ${5thDlibs}/libsqlcipher.so)


message("ZMQ_LIB full path: ${ZMQ_LIB}")
message("SQLCIPHER_LIB full path: ${SQLCIPHER_LIB}")

add_library(zmq SHARED IMPORTED)
set_target_properties(zmq PROPERTIES IMPORTED_LOCATION ${ZMQ_LIB})

add_library(sqlcipher SHARED IMPORTED)
set_target_properties(sqlcipher PROPERTIES IMPORTED_LOCATION ${SQLCIPHER_LIB})

file(GLOB PEER_FILES 
    "core/src/*.cpp"
    "../core/receiver.cpp"
    "../core/izmq.cpp"
    "../core/5thdlogger.cpp"
    "../core/5thdipcmsg.c"
    "../core/5thdsql.cpp"
    "../core/keys_db.cpp"
    "../core/transmitter.cpp"
    "../core/5thdipc_client.cpp"
    "../core/module.cpp"
    "../core/5thdallocator.cpp"

)

set(SOURCES peer_module.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${PEER_FILES})

target_link_libraries(${PROJECT_NAME}
    spdlog::spdlog
    sodium
    zmq
    sqlcipher
)
