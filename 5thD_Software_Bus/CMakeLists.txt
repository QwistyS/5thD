cmake_minimum_required(VERSION 3.20)
project(5thDSoftwareBus)

# Include directories for core module headers
include_directories(
    core/inc
)

file(GLOB SOFT_BUS_FILES 
    "core/src/*.cpp"
    "../core/izmq.cpp"
    "../core/5thdallocator.cpp"
    "../core/5thdlogger.cpp"
    "../core/5thdipcmsg.c"
    "../core/5thdsql.cpp"
    "../core/keys_db.cpp"
    "../core/module.cpp"

)

set(SOURCES soft_bus.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${SOFT_BUS_FILES})

target_link_libraries(${PROJECT_NAME}
    sodium
    zmq
    sqlcipher
    tracy
)