cmake_minimum_required(VERSION 3.20)
project(5thDPeer)

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
    spdlog
    sodium
    zmq
    sqlcipher
)
