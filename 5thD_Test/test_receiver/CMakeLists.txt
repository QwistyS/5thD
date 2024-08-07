cmake_minimum_required(VERSION 3.20)
project(5thDreceiverTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path for the library files
set(ZMQ_LIB ${5thDlibs}/libzmq${LIB_EXTENSION})
message("ZMQ_LIB full path: ${ZMQ_LIB}")

add_library(zmq SHARED IMPORTED)
set_target_properties(zmq PROPERTIES IMPORTED_LOCATION ${ZMQ_LIB})


file(GLOB TESTS_RECEIVER
    "../../core/5thdlogger.cpp"
    "../../core/izmq.cpp"
    "../../core/5thdipcmsg.c"
    "../../core/receiver.cpp"
)


set(SOURCES test_all.cpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${TESTS_RECEIVER})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog 
    sodium
    unity
    zmq
)
